
#include "App.h"

// sound stuff
#include "AudioSystem.h"
#include "TouchServiceProvider.h"
#include "TouchAudioFormat.h"
#include "AudioOutput.h"
#include "Wavetable.h"

// ui stuff
#include "Settings.h"
#include "ofxiPhoneExtras.h"
#include "UIActionHandler.h"
#include <math.h>

App* gApp = NULL;

// barebones UIViewController so we can control allowed orientations of SoundCloud interface.
@interface LandscapeViewController : UIViewController 
{
}
@end

@implementation LandscapeViewController

- (BOOL) shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation
{
    if ( UIInterfaceOrientationIsLandscape(toInterfaceOrientation) )
    {
        return YES;
    }

    return NO;
}

- (void) willRotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation duration:(NSTimeInterval)duration
{
    // snap to new rotation
    [UIView setAnimationsEnabled:NO];
}

- (void)didRotateFromInterfaceOrientation:(UIInterfaceOrientation)fromInterfaceOrientation 
{
    // animations back on so we get the nice slide-ons for the keyboard and so forth
    [UIView setAnimationsEnabled:YES];
}

@end

// global tweaks
const int   kOutputBufferSize = 1024;
const int   kStreamBufferSize = 512;
const int   kSpectralGenSize  = 1024 * 8;

float kMaxSpectralAmp   = 128.0f;
int   kToolbarHeight    = 60;

// not actually constant because it needs to be set relative to actual width
float kFirstBandInset       = 30;

int       lastBand        = 1;
float     stringAnimation = 0;

void computeLastBand()
{
    lastBand = ofClamp( Settings::BandOffset + 128*Settings::BandSpacing, 3, kSpectralGenSize/4 );
}

void bandSpacingChanged( float value )
{
    Settings::BandSpacing = value;
    computeLastBand();
}

void bandOffsetChanged( float value )
{
    Settings::BandOffset = value;
    computeLastBand();
}

void decayChanged( float value )
{
    Settings::Decay = value;
}

void bitCrushChanged( float value )
{
    Settings::BitCrush = value;
}

void pitchChanged( float value )
{
    Settings::Pitch = value;
}

//--------------------------------------------------------------
App::App()
: ofxiPhoneApp()
, specGen(kSpectralGenSize)
, bitCrush(24, Settings::BitCrush)
, tickRate( Settings::Pitch )
, highPass( 30, 0, Minim::MoogFilter::HP )
, mSliderWidth(200 * (ofGetWidth() / 1024.0f))
, mBandSpacingSlider( "",
                     mSliderWidth*0.6f, ofGetHeight()-25, // position
                     mSliderWidth, 30, // size
                     120, // hue
                     Settings::BandSpacing, Settings::BandSpacingMin, Settings::BandSpacingMax, 
                     bandSpacingChanged )
, mBandOffsetSlider( "",
                     mSliderWidth*0.6f + mSliderWidth*1.25f, ofGetHeight()-25, //position
                     mSliderWidth, 30, // size
                     120, //hue
                    Settings::BandOffset, Settings::BandOffsetMin, Settings::BandOffsetMax,
                    bandOffsetChanged )
, mBandDecaySlider( "",
                   mSliderWidth*0.6f + mSliderWidth*1.25f*2, ofGetHeight()-25, //position
                   mSliderWidth, 30, //size
                   120, //hue
                   Settings::Decay, Settings::DecayMin, Settings::DecayMax,
                   decayChanged )
, mBitCrushSlider( "",
                   mSliderWidth*0.6f + mSliderWidth*1.25f*3, ofGetHeight()-25, // position
                   mSliderWidth, 30, // size
                   120, // hue
                   Settings::BitCrush, Settings::BitCrushMin, Settings::BitCrushMax,
                   bitCrushChanged )
, mPitchSlider   ( "",
                    mSliderWidth*0.6f + mSliderWidth*1.25f, ofGetHeight()-25, //position
                    mSliderWidth, 30, // size
                    120, //hue
                    Settings::Pitch, Settings::PitchMin, Settings::PitchMax,
                    pitchChanged )
{
	gApp = this;
}

//--------------------------------------------------------------
void App::setup()
{	
	mAudioSystem = NULL;
	mOutput = NULL;
	
	ofSetBackgroundAuto(false);
	
	ofEnableAlphaBlending();
    
    ofEnableSmoothing();
	
	// register touch events
	ofRegisterTouchEvents(this);
	
	// initialize the accelerometer
	ofxAccelerometer.setup();
	
	// iPhoneAlerts will be sent to this.
	ofxiPhoneAlerts.addListener(this);
	
	// might want to do this at some point.
	ofxiPhoneSetOrientation( OF_ORIENTATION_90_RIGHT );
	
	//-- AUDIO --------------------------------------
	{
		TouchServiceProvider::AudioSessionParameters sessionParams;
		sessionParams.outputBufferDuration = (float)kOutputBufferSize / 44100.f;
		
		extern void AudioInterruptionListener ( void *inClientData, UInt32 inInterruptionState );
		sessionParams.interruptListener = &AudioInterruptionListener;
		sessionParams.interruptUserData = this;
		
		mAudioSystem = new Minim::AudioSystem( new TouchServiceProvider(sessionParams) );
		
		extern void AudioRouteChangeListener( void *inClientData, AudioSessionPropertyID inID, UInt32 inPropertySize, const void *inPropertyValue );
		AudioSessionAddPropertyListener(kAudioSessionProperty_AudioRouteChange, &AudioRouteChangeListener, this);
		
		// turn off the wavetable optimisation that skips interpolation.
		Minim::Wavetable::s_opt = false;
		
		mOutput = mAudioSystem->getAudioOutput( TouchAudioFormat(2), kOutputBufferSize );
        
        computeLastBand();
        
        tickRate.value.setLastValue( Settings::Pitch );
        tickRate.setInterpolation( true );
        
        specGen.patch( tickRate ).patch( bitCrush ).patch( *mOutput );
	}
	
	//-- UI ---------------------------
	{
        kFirstBandInset = 30 * (ofGetWidth()/1024.f);
        
		mActionHandler = [[UIActionHandler alloc] init:this];
        
        // remove the glview from the UIWindow, which is where openFrameworks puts it
        [ofxiPhoneGetGLView() removeFromSuperview];

        // create a plain old UIView that is the same size as the app
        // just use OF sizes as our reference, everything should work out
        CGRect mainFrame = CGRectMake(0, 0, ofGetWidth(), ofGetHeight());
        UIView* mainView = [[UIView alloc] initWithFrame:mainFrame];

        // grab the glView and rotate/position it so it looks correct
        UIView* glView      = ofxiPhoneGetGLView();
        glView.transform    = CGAffineTransformRotate(CGAffineTransformIdentity, M_PI_2);
        glView.center       = mainView.center;

        // glView goes on our main view to prevent wonky resizing
        [mainView addSubview:glView];

        // root controller has main view as its view
        LandscapeViewController* rootViewController = [[LandscapeViewController alloc] init];
        rootViewController.view                     = mainView;

        // and finally we set the root view controller
        // which will make mMainView the window's primary view
        // all other views are then added as subviews of mMainView
        // NOT as subviews of the UIWindow or the EAGLView
        ofxiPhoneGetUIWindow().rootViewController = rootViewController;
		
		// action sheet shown sometimes
        if ( 0 )
		{
			NSString * actionTitle = NSLocalizedString(@"A false truth?", @"");
			NSString * actionYes   = NSLocalizedString(@"Yes", @"");
			NSString * actionNo    = NSLocalizedString(@"Yes", @"");
			mActionSheet = [[UIActionSheet alloc] initWithTitle:actionTitle delegate:mActionHandler cancelButtonTitle:nil destructiveButtonTitle:actionNo otherButtonTitles:actionYes,nil];
			mActionSheet.actionSheetStyle = UIActionSheetStyleBlackTranslucent;
			mActionSheet.delegate = mActionHandler;
		}
		
		// pinch gesture mostly for zooming in and out
        if ( 0 )
		{
			mPinchGestureRecognizer = [[UIPinchGestureRecognizer alloc] initWithTarget:mActionHandler action:@selector(handlePinchGesture:)];
			[ofxiPhoneGetUIWindow() addGestureRecognizer:mPinchGestureRecognizer];
		}
        
        // slider labels
        // spacing
        {
            Box& box = mBandSpacingSlider.box();
            CGRect frame    = CGRectMake(box.mMinX, box.mMinY-20, box.mW, 20);
            UILabel* label = [[UILabel alloc] initWithFrame:frame];
            label.font     = [UIFont systemFontOfSize:14];
            label.text     = @"Spacing";
            label.textColor = [UIColor grayColor];
            label.shadowColor = [UIColor colorWithWhite:0.2f alpha:1.0f];
            label.shadowOffset = CGSizeMake(1,1);
            label.backgroundColor = [UIColor clearColor];
            [mainView addSubview:label];
        }
        
        // root
        if ( 0 )
        {
            Box& box = mBandOffsetSlider.box();
            CGRect frame    = CGRectMake(box.mMinX, box.mMinY-20, box.mW, 20);
            UILabel* label = [[UILabel alloc] initWithFrame:frame];
            label.font     = [UIFont systemFontOfSize:14];
            label.text     = @"Pitch";
            label.textColor = [UIColor grayColor];
            label.shadowColor = [UIColor colorWithWhite:0.2f alpha:1.0f];
            label.shadowOffset = CGSizeMake(1,1);
            label.backgroundColor = [UIColor clearColor];
            [mainView addSubview:label];
        }
        
        // decay
        {
            Box& box = mBandDecaySlider.box();
            CGRect frame    = CGRectMake(box.mMinX, box.mMinY-20, box.mW, 20);
            UILabel* label = [[UILabel alloc] initWithFrame:frame];
            label.font     = [UIFont systemFontOfSize:14];
            label.text     = @"Decay";
            label.textColor = [UIColor grayColor];
            label.shadowColor = [UIColor colorWithWhite:0.2f alpha:1.0f];
            label.shadowOffset = CGSizeMake(1,1);
            label.backgroundColor = [UIColor clearColor];
            [mainView addSubview:label];
        }
        
        // bit crush
        {
            Box& box = mBitCrushSlider.box();
            CGRect frame    = CGRectMake(box.mMinX, box.mMinY-20, box.mW, 20);
            UILabel* label = [[UILabel alloc] initWithFrame:frame];
            label.font     = [UIFont systemFontOfSize:14];
            label.text     = @"Crush";
            label.textColor = [UIColor grayColor];
            label.shadowColor = [UIColor colorWithWhite:0.2f alpha:1.0f];
            label.shadowOffset = CGSizeMake(1,1);
            label.backgroundColor = [UIColor clearColor];
            [mainView addSubview:label];
        }
        
        // pitch
        {
            Box& box = mPitchSlider.box();
            CGRect frame    = CGRectMake(box.mMinX, box.mMinY-20, box.mW, 20);
            UILabel* label = [[UILabel alloc] initWithFrame:frame];
            label.font     = [UIFont systemFontOfSize:14];
            label.text     = @"Pitch";
            label.textColor = [UIColor grayColor];
            label.shadowColor = [UIColor colorWithWhite:0.2f alpha:1.0f];
            label.shadowOffset = CGSizeMake(1,1);
            label.backgroundColor = [UIColor clearColor];
            [mainView addSubview:label];
        }
	}
	
	//-- DONE ---------------------------
	m_bWasPlaying			= false;
}

float expoEaseOut( float t, float b, float c, float d )
{
	return c * ( -powf( 2, -10 * t/d ) + 1 ) + b;
}

//--------------------------------------------------------------
void App::update() 
{
    const float dt  = 1.0f / ofGetFrameRate();
    
    stringAnimation += dt * TWO_PI * 2;
    if ( stringAnimation > TWO_PI )
    {
        stringAnimation -= TWO_PI;
    }
    
    for( int b = 0; b < kSpectralGenSize/2; ++b )
    {
        float m = ofClamp( specGen.getBandMagnitude(b) * ofRandom( Settings::Decay - 0.3f, Settings::Decay), 0, kMaxSpectralAmp );
        specGen.setBandMagnitude(b, m);
        //specGen.setBandPhase( b, specGen.getBandPhase(b) + ofRandom(M_PI/24, M_PI_4) );
    }
    
    float t = ofMap(Settings::BitCrush, Settings::BitCrushMin, Settings::BitCrushMax, 0, 1);
    float crush = expoEaseOut( t, Settings::BitCrushMin, Settings::BitCrushMax - Settings::BitCrushMin, 1 );
    //printf( "crush with rate %f and depth %f\n", crush, bitCrush.bitRes.getLastValue() );
    bitCrush.bitRate.setLastValue( crush );
    
    tickRate.value.setLastValue( Settings::Pitch );
}

//--------------------------------------------------------------
void App::draw() 
{	
    ofBackground(20, 20, 20);
    
    for( int b = Settings::BandOffset; b < lastBand; b += Settings::BandSpacing )
    {
        float x = ofMap( b, Settings::BandOffset, lastBand, kFirstBandInset, ofGetWidth() - kFirstBandInset );
        float p = specGen.getBandPhase(b) + stringAnimation;
        float m = specGen.getBandMagnitude(b);
        
        float br = ofMap( m, 0, kMaxSpectralAmp, 0.4f, 1 );
        ofSetColor( 255.f * br );
        
        float w = ofMap( m, 0, kMaxSpectralAmp, 0, 6 );
        float segLength = ofGetHeight()/32;
        ofPolyline string;
        for( float y = 0; y < ofGetHeight(); y += segLength )
        {
            float s1 = y/ofGetHeight() * M_PI * 8 + p;
            //float s2 = (y+segLength)/ofGetHeight() * M_PI * 8 + p;
            string.addVertex( ofPoint(x + w*sinf(s1),y) );
            //ofLine( x + w*sinf(s1), y, x + w*sinf(s2), y + segLength );
        }
        
        string.draw();
    }
    
    ofEnableAlphaBlending();
        
    ofSetColor(10, 10, 10, 192);
    ofRect(0, ofGetHeight()-kToolbarHeight-5, ofGetWidth(), ofGetHeight()-kToolbarHeight);
    
    ofBeginShape();
    {
        ofSetColor(30);
        ofVertex(0, ofGetHeight()-kToolbarHeight);
        ofVertex(ofGetWidth(), ofGetHeight()-kToolbarHeight);
        ofVertex(ofGetWidth(), ofGetHeight());
        ofVertex(0, ofGetHeight());
    }
    ofEndShape();
    
    mBandSpacingSlider.draw( );
    //mBandOffsetSlider.draw( );
    mBandDecaySlider.draw( );
    mBitCrushSlider.draw();
    mPitchSlider.draw();
        
    
//    ofSetColor(255, 255, 255);
//    string fps("FPS: ");
//    fps += ofToString( ofGetFrameRate() );
//    ofDrawBitmapString(fps, 5, 15);
}

//--------------------------------------------------------------
void App::exit() 
{
	[mActionHandler release];
    
    specGen.unpatch( *mOutput );
	
	if ( mOutput )
	{
		mOutput->close();
		delete mOutput;
	}
	
	if ( mAudioSystem )
	{
		delete mAudioSystem;
	}
}


