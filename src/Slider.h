//
//  Slider.h
//  SpectralHarp
//
//  Created by Damien Di Fede on 5/6/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef SpectralHarp_Slider_h
#define SpectralHarp_Slider_h

#include "Box.h"
#include "ofMain.h"

class Slider
{
public:
    Slider()
    : mLabel( "" )
    , mBox(0,0,0,0)
    {
    }
    
    Slider( string label, float x, float y, float w, float h, int hue, float defaultv, float minv, float maxv, void(*ValueChanged)(float) )
    : mLabel( label )
    , mBox( x, y, w, h )
    , mHue(hue)
    , mValue(defaultv)
    , mMinValue(minv)
    , mMaxValue(maxv)
    , mTouch(-1)
    , mValueChangedCallback(ValueChanged)
    {
    }
    
    void draw();
    
    Box& box() { return mBox; }
    
    // returns true if handled
    bool handleTouchDown( const int id, const float x, const float y );
    bool handleTouchMoved( const int id, const float x, const float y );
    bool handleTouchUp( const int id, const float x, const float y );
    
private:
    
    string  mLabel;
    int     mHue;
    Box     mBox;
    float   mValue;
    float   mMinValue;
    float   mMaxValue;
    int     mTouch; // the touch we are currently tracking
    
    void(*mValueChangedCallback)(float);
};


#endif
