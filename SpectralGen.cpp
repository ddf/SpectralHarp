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

SpectralGen::SpectralGen( const int inTimeSize )
: UGen()
, decayRate( *this, CONTROL, 0 )
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
	
	reset();
}

SpectralGen::~SpectralGen()
{
    delete [] specReal;
    delete [] specImag;
    delete [] inverse;
    delete [] output;
}

void SpectralGen::reset()
{
	memset(specReal, 0, sizeof(float)*timeSize);
	memset(specImag, 0, sizeof(float)*timeSize);
	memset(inverse, 0, sizeof(float)*timeSize);
	memset(output, 0, sizeof(float)*timeSize);
	
	for(int i = 0; i < specSize; ++i)
	{
		bands[i].struckAmplitude = 0;
		bands[i].amplitude = 0;
	}
	
	outIndex = 0;
}

void SpectralGen::uGenerate(float* out, const int numChannels)
{
    if ( outIndex % windowSize == 0 )
    {
		// ddf (5/12/17)
		// now using the same phase for every band because this prevents some of the clicking.
		// evolving the phase over time never added much to the sound anyhow.
		const float phase = (float)M_PI*1.5f;
		const float cosP = cosf(phase);
		const float sinP = sinf(phase);

		// DQ (2/20/18)
		// now pull the decay from a UGenInput and pass it in to the band struct instead of using a static var.
		const float decay = decayRate.getLastValue();

        // this one outside the loop so we don't need an if check around the "top half" assign
        bands[0].update(decay);
        specReal[0] = bands[0].amplitude*cosP;
        specImag[0] = bands[0].amplitude*sinP;
		specReal[specSize] = specReal[0];
		specImag[specSize] = -specImag[0];		
        
        for( unsigned i = 1; i < specSize; ++i )
        {
            band& b = bands[i];
            b.update(decay);
            
            specReal[i] = b.amplitude*cosP;
            specImag[i] = b.amplitude*sinP;
            
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
