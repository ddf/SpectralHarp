//
//  SpectralGen.cpp
//  SpectralHarp
//
//  Created by Damien Di Fede on 5/6/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include "SpectralGen.h"
#include "FourierTransform.h"
#include <stdio.h>
#include <string> // for memset

float SpectralGen::decay(0);

SpectralGen::SpectralGen( const int inTimeSize )
: UGen()
, timeSize(inTimeSize)
, specSize(inTimeSize/2)
, outIndex(0)
, windowSize(inTimeSize/2)
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
		// ddf (5/12/17)
		// now using the same phase for every band because this prevents some of the clicking.
		// evolving the phase over time never added much to the sound anyhow.
		const float phase = (float)M_PI*1.5f;
        // this one outside the loop so we don't need an if check around the "top half" assign
        bands[0].update();
        specReal[0] = bands[0].amplitude*cosf(phase);
        specImag[0] = bands[0].amplitude*sinf(phase);
		specReal[specSize] = specReal[0];
		specImag[specSize] = -specImag[0];
        
        for( unsigned i = 1; i < specSize; ++i )
        {
            band& b = bands[i];
            b.update();
            
            specReal[i] = b.amplitude*cosf(phase);
            specImag[i] = b.amplitude*sinf(phase);
            
            // and the top half
			specReal[timeSize - i] = specReal[i];
			specImag[timeSize - i] = -specImag[i];
        }
        
        fft.Minim::FourierTransform::inverse(specReal, specImag, inverse);
        Minim::FourierTransform::HANN.apply( inverse, timeSize );
        
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
