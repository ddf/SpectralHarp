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

//--------------------------------------------------------------
void App::touchDown(ofTouchEventArgs &touch)
{
    if ( mBandSpacingSlider.handleTouch(touch.id, touch.x, touch.y) )
        return;
    
    if ( mBandOffsetSlider.handleTouch(touch.id, touch.x, touch.y) )
        return;
    
    if ( mBandDecaySlider.handleTouch(touch.id, touch.x, touch.y) )
        return;
    
    if ( touch.y > ofGetHeight()-kToolbarHeight )
        return;
    
    prevX[touch.id] = touch.x;
    prevY[touch.id] = touch.y;
}

//--------------------------------------------------------------
void App::touchMoved(ofTouchEventArgs &touch)
{
    if ( mBandSpacingSlider.handleTouch(touch.id, touch.x, touch.y) )
        return;
    
    if ( mBandOffsetSlider.handleTouch(touch.id, touch.x, touch.y) )
        return;
    
    if ( mBandDecaySlider.handleTouch(touch.id, touch.x, touch.y) )
        return;
    
    if ( touch.y > ofGetHeight()-kToolbarHeight )
        return;
    
    for( int b = Settings::BandOffset; b < lastBand; b += Settings::BandSpacing )
    {
        float x = ofMap( b, Settings::BandOffset, lastBand, 10, ofGetWidth()-10 );
        if ( (prevX[touch.id] < x && touch.x > x) || (touch.x < x && prevX[touch.id] > x) )
        {
            float mag = ofMap(touch.y, ofGetHeight(), 0, 0, kMaxSpectralAmp);
            specGen.setBandMagnitude( b, mag );
        }
    }
    
    prevX[touch.id] = touch.x;
    prevY[touch.id] = touch.y;
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