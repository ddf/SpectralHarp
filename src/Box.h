//
//  Box.h
//  melodizer
//
//  Created by Damien Di Fede on 12/14/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#ifndef melodizer_Box_h
#define melodizer_Box_h

struct Box
{
    Box( float cx, float cy, float w, float h )
    : mX(cx)
    , mY(cy)
    , mW(w)
    , mH(h)
    , mMinX( mX - mW/2 )
    , mMaxX( mX + mW/2 )
    , mMinY( mY - mH/2 )
    , mMaxY( mY + mH/2 )
    {
        
    }
    
    float   mX, mY, mW, mH;
    float   mMinX, mMaxX, mMinY, mMaxY;
    
    // position the box using the top left corner
    inline void setCorner( const float x, const float y )
    {
        mX = x + mW/2;
        mY = y + mH/2;
        mMinX = x;
        mMaxX = x + mW;
        mMinY = y;
        mMaxY = y + mH;
    }
    
    // position the box using the center point
    inline void setCenter( const float cx, const float cy )
    {
        mX = cx;
        mY = cy;
        mMinX = mX - mW/2;
        mMaxX = mX + mW/2;
        mMinY = mY - mH/2;
        mMaxY = mY + mH/2;
    }
    
    inline bool contains( float x, float y )
    {
        return (x > mMinX && x < mMaxX && y > mMinY && y < mMaxY);
    }
};

#endif
