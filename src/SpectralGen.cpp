//
//  SpectralGen.cpp
//  SpectralHarp
//
//  Created by Damien Di Fede on 5/6/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include "SpectralGen.h"
#include "FourierTransform.h"
#include "Settings.h"
#include "ofMath.h"
#include <stdio.h>
#include <string> // for memset

float SpectralGen::decay(0);

SpectralGen::SpectralGen( const int inTimeSize )
: UGen()
, timeSize(inTimeSize)
, specSize(inTimeSize/2)
, outIndex(0)
, windowSize(inTimeSize/4)
, fft(inTimeSize,44100)
{
    specReal = new float[timeSize];
    specImag = new float[timeSize];
    inverse  = new float[timeSize];
    output   = new float[timeSize];
}

SpectralGen::~SpectralGen()
{
    delete [] specReal;
    delete [] specImag;
    delete [] inverse;
    delete [] output;
}

void SpectralGen::uGenerate(float* out, const int numChannels)
{
    if ( outIndex % windowSize == 0 )
    {
        // this one outside the loop so we don't need an if check around the "top half" assign
        bands[0].update();
        specReal[0] = bands[0].amplitude*cosf(bands[0].phase);
        specImag[0] = bands[0].amplitude*sinf(bands[0].phase);
        
        for( unsigned i = 1; i < specSize; ++i )
        {
            band& b = bands[i];
            b.update();
            
            specReal[i] = b.amplitude*cosf(b.phase);
            specImag[i] = b.amplitude*sinf(b.phase);
            
            // and the top half
            specReal[timeSize - i] = specReal[i];
            specImag[timeSize - i] = -specImag[i];
        }
        
        fft.Minim::FourierTransform::inverse(specReal, specImag, inverse);
        Minim::FourierTransform::HAMMING.apply( inverse, timeSize );
        
        for( int s = 0; s < timeSize; ++s )
        {
            int ind = (s + outIndex) % timeSize;
            
            output[ind] += inverse[s];
            
            // soft-clip
//            static float clipFactor = 2;
//            static float invClipFactor = 1.f / clipFactor;
//            output[ind] = invClipFactor * atanf( clipFactor * output[ind] );
        }
    }
    
    UGen::fill(out, output[outIndex], numChannels);
    output[outIndex] = 0;
    
    outIndex = (outIndex+1)%timeSize;
}