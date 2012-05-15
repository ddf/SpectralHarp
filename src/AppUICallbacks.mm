/*
 *  AppUICallbacks.mm
 *  noiseShaper
 *
 *  Created by Damien Di Fede on 6/25/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "App.h"

#include "ofxiPhoneExtras.h"
#include "Settings.h"

extern int lastBand;
extern float kMaxSpectralAmp;
extern int   kToolbarHeight;
extern float kFirstBandInset;

//--------------------------------------------------------------
void App::touchDown(ofTouchEventArgs &touch)
{
    if ( mBandSpacingSlider.handleTouchDown(touch.id, touch.x, touch.y) )
        return;
    
    if ( mBandOffsetSlider.handleTouchDown(touch.id, touch.x, touch.y) )
        return;
    
    if ( mBandDecaySlider.handleTouchDown(touch.id, touch.x, touch.y) )
        return;
    
    if ( touch.y > ofGetHeight()-kToolbarHeight )
        return;
    
    prevTouch[touch.id] = touch;
}

//--------------------------------------------------------------
void App::touchMoved(ofTouchEventArgs &touch)
{
    if ( mBandSpacingSlider.handleTouchMoved(touch.id, touch.x, touch.y) )
        return;
    
    if ( mBandOffsetSlider.handleTouchMoved(touch.id, touch.x, touch.y) )
        return;
    
    if ( mBandDecaySlider.handleTouchMoved(touch.id, touch.x, touch.y) )
        return;
    
    if ( touch.y > ofGetHeight()-kToolbarHeight )
        return;
    
    for( int b = Settings::BandOffset; b < lastBand; b += Settings::BandSpacing )
    {
        float x = ofMap( b, Settings::BandOffset, lastBand, kFirstBandInset, ofGetWidth()-kFirstBandInset );
        if ( (prevTouch[touch.id].x < x && touch.x > x) || (touch.x < x && prevTouch[touch.id].x > x) )
        {
            float speed = fabs(touch.x - prevTouch[touch.id].x) / (ofGetElapsedTimeMillis() - prevTouch[touch.id].time);
            float mag   = ofMap(speed, 0, 10, kMaxSpectralAmp*0.2f, kMaxSpectralAmp, true);
            specGen.setBandMagnitude( b, mag );
            float ps    = ofMap( touch.y, 0, ofGetHeight(), M_PI/24, M_PI_2, true );
            specGen.setBandPhaseStep( b, ps );
        }
    }
                                                             
    prevTouch[touch.id] = touch;
    prevTouch[touch.id].time = ofGetElapsedTimeMillis();
}

//--------------------------------------------------------------
void App::touchUp(ofTouchEventArgs &touch)
{
    if ( mBandSpacingSlider.handleTouchUp(touch.id, touch.x, touch.y) )
        return;
    
    if ( mBandOffsetSlider.handleTouchUp(touch.id, touch.x, touch.y) )
        return;
    
    if ( mBandDecaySlider.handleTouchUp(touch.id, touch.x, touch.y) )
        return;
}

//--------------------------------------------------------------
void App::touchDoubleTap(ofTouchEventArgs &touch)
{
    
}

//--------------------------------------------------------------
void App::touchCancelled(ofTouchEventArgs &touch)
{
    if ( mBandSpacingSlider.handleTouchUp(touch.id, touch.x, touch.y) )
        return;
    
    if ( mBandOffsetSlider.handleTouchUp(touch.id, touch.x, touch.y) )
        return;
    
    if ( mBandDecaySlider.handleTouchUp(touch.id, touch.x, touch.y) )
        return; 
}

//--------------------------------------------------------------
void App::handlePinchGesture()
{
}