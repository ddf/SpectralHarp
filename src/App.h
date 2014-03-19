#pragma once

#include "ofMain.h"
#include "ofxiPhone.h"
#include "SpectralGen.h"
#include "BitCrush.h"
#include "TickRate.h"
#include "MoogFilter.h"
#include "Slider.h"

// forward declares
@class UIActionHandler;
@class UIActionSheet;

namespace Minim 
{
	class AudioSystem;
	class AudioOutput;
}


class App : public ofxiOSApp
{
	
public:
	App();
	
	void setup();
	void update();
	void draw();
	void exit();
	
	void touchDown(ofTouchEventArgs &touch);
	void touchMoved(ofTouchEventArgs &touch);
	void touchUp(ofTouchEventArgs &touch);
	void touchDoubleTap(ofTouchEventArgs &touch);
	void touchCancelled(ofTouchEventArgs &touch);
	
	Minim::AudioOutput & Out() { return *mOutput; }
	
	void pauseAudio();
	void resumeAudio();
	void lostFocus();
	void gotFocus();
	void gotMemoryWarning();
	void deviceOrientationChanged(int newOrientation);
	void handlePinchGesture();
	
private:
	
	// INPUT
	UIPinchGestureRecognizer *	mPinchGestureRecognizer;
    ofTouchEventArgs prevTouch[32];
	
	// SOUND
	Minim::AudioSystem *	  mAudioSystem;
	Minim::AudioOutput *	  mOutput;
    
    SpectralGen               specGen;
    Minim::BitCrush           bitCrush;
    Minim::TickRate           tickRate;
    Minim::MoogFilter         highPass;
	
	// UI
	UIActionHandler				 * mActionHandler;
	UIActionSheet			     * mActionSheet; // so we can ask things
    float                          mSliderWidth;
    Slider                         mBandSpacingSlider;
    Slider                         mBandOffsetSlider;
    Slider                         mBandDecaySlider;
    Slider                         mBitCrushSlider;
    Slider                         mPitchSlider;
    ofTrueTypeFont                 mFont;
    ofPolyline                     mString;
	
	bool						   m_bWasPlaying; // keep track of whether we were playing or not when audio got paused.
};

