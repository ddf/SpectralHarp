//
//  Slider.cpp
//  SpectralHarp
//
//  Created by Damien Di Fede on 5/6/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include "Slider.h"
#include "ofMain.h"

/////////////////////////////////////////
//
// SLIDER 
// 
/////////////////////////////////////////
void Slider::draw( )
{
    // label
    {
        ofSetColor(200, 200, 200);
        ofDrawBitmapString( mLabel, mBox.mMinX, mBox.mMinY-5 );
    }
    
    // background
    {
        ofSetColor(60, 60, 60);
        ofRect( mBox.mMinX, mBox.mMinY, mBox.mW, mBox.mH );
    }
    
    // fill
    {        
        const float normValue = ofNormalize(mValue, mMinValue, mMaxValue);
        // adjust brightness based on amt of fill
        float bright = ( 64 + 190 * normValue );
        ofColor color;
        
        if ( mHue >= 0 )
        {
            color.setHsb( mHue, 255, 96 );
        }
        else
        {
            color.set( bright, bright, bright );
        }
        
        ofSetColor(color);
        
        float fW = fmax( mBox.mW * normValue, 1 );
        
        ofRect( mBox.mMinX, mBox.mMinY, fW, mBox.mH );
    }
}

//----------------------------------
bool Slider::handleTouchDown(const int id, const float x, const float y)
{
    // if inside our box, start tracking
    const bool bInside = mBox.contains( x, y ); 
    if ( bInside && mTouch == -1 )
    {
        //printf( "Value Slider started tracking %d.\n", id );
        mTouch = id;
        
        // apply value
        handleTouchMoved(id, x, y);
    }
    
    return bInside;
}

//----------------------------------
bool Slider::handleTouchMoved(const int id, const float x, const float y)
{
    bool bTracked = (id == mTouch);
    
    if ( bTracked )
    {
        mValue = ofMap( (x - mBox.mMinX) / mBox.mW, 0, 1, mMinValue, mMaxValue, true );
        mValueChangedCallback(mValue);
    }
    
    return bTracked;
}

//----------------------------------
bool Slider::handleTouchUp( const int id, const float x, const float y )
{
    if ( id == mTouch )
    {
        //printf( "Value Slider stopped tracking %d.\n", id );
        mTouch = -1;
        return true;
    }
    
    return false;
}