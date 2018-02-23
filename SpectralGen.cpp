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

enum
{
	kSpectralMagnitude = 1024
};

SpectralGen::SpectralGen( const int inTimeSize )
: UGen()
, decayRate( *this, CONTROL, 0 )
, inverseSize(inTimeSize)
, specSize(inTimeSize/2)
, outputSize(inTimeSize*2)
, outIndex(0)
, overlapSize(inTimeSize/2)
, fft(inTimeSize,44100)
{
    specReal = new float[inverseSize];
    specImag = new float[inverseSize];
    inverse  = new float[inverseSize];
    output   = new float[outputSize];
	
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
	memset(specReal, 0, sizeof(float)*inverseSize);
	memset(specImag, 0, sizeof(float)*inverseSize);
	memset(inverse, 0, sizeof(float)*inverseSize);
	memset(output, 0, sizeof(float)*outputSize);
	
	for(int i = 0; i < specSize; ++i)
	{
		bands[i].struckAmplitude = 0;
		bands[i].amplitude = 0;
		bands[i].phase = 0;
		bands[i].phaseStep = M_PI*(i%2);
	}
	
	outIndex = 0;
}

void SpectralGen::sampleRateChanged()
{
}

void SpectralGen::pluck(const int b, const float amp)
{
	bands[b].pluck(amp);
}

void SpectralGen::uGenerate(float* out, const int numChannels)
{
    if ( outIndex % overlapSize == 0 )
    {
		// DQ (2/20/18)
		// now pull the decay from a UGenInput and pass it in to the band struct instead of using a static var.
		const float decay = decayRate.getLastValue();
		
        for( unsigned i = kBandMin; i <= kBandMax; ++i )
        {
            band& b = bands[i];
            b.update(decay);
			
			const float x = b.amplitude*cosf(bands[i].phase);
			const float y = b.amplitude*sinf(bands[i].phase);
			
			specReal[i] = kSpectralMagnitude*x;
			specImag[i] = kSpectralMagnitude*y;
        }
        
        fft.Minim::FourierTransform::inverse(specReal, specImag, inverse);
        Minim::FourierTransform::TRIANGULAR.apply( inverse, inverseSize );
		
        for( int s = 0; s < inverseSize; ++s )
        {
            int ind = (s + outIndex) % outputSize;
            
            output[ind] += inverse[s];
        }
    }
    
    UGen::fill(out, output[outIndex], numChannels);
    output[outIndex] = 0;
    
    outIndex = (outIndex+1)%outputSize;
}
