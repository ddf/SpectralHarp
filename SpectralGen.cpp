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
, decay( *this, CONTROL, 0 )
, inverseSize(inTimeSize)
, specSize(inTimeSize/2)
, outputSize(inTimeSize*2)
, outIndex(0)
, overlapSize(inTimeSize/2)
, fft(inTimeSize,44100)
{
	bands    = new band[specSize];
    specReal = new float[inverseSize];
    specImag = new float[inverseSize];
    inverse  = new float[inverseSize];
    output   = new float[outputSize];
	
	reset();
}

SpectralGen::~SpectralGen()
{
	delete [] bands;
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
		bands[i].amplitude = 0;
		bands[i].phase = 0;
		bands[i].phaseStep = M_PI*(i%2);
	}
	
	outIndex = 0;
}

void SpectralGen::sampleRateChanged()
{
	fft.setSampleRate(sampleRate());
}

void SpectralGen::pluck(const float freq, const float amp)
{
	const int b = fft.freqToIndex(freq);
	if ( b >= 0 && b < specSize )
	{
		bands[b].amplitude = amp;
		bands[b].decay = 1;
	}
}

float SpectralGen::getBandPhase(const float freq) const
{
	const int b = fft.freqToIndex(freq);
	return b>=0 && b<specSize ? bands[b].phase : 0;
}

float SpectralGen::getBandMagnitude(const float freq) const
{
	const int b = fft.freqToIndex(freq);
	return b>=0 && b<specSize ? bands[b].amplitude*bands[b].decay : 0;
}

void SpectralGen::uGenerate(float* out, const int numChannels)
{
    if ( outIndex % overlapSize == 0 )
    {
		// DQ (2/20/18)
		// now pull the decay from a UGenInput and pass it in to the band struct instead of using a static var.
		// this is in seconds, we need to convert to a fixed amount we can subtract from each band's amplitude,
		// which depends on sample rate and overlap size.
		const float decaySeconds = decay.getLastValue();
		// amplitude needs to decrease by 1 / (decaySeconds * sampleRate()) every sample.
		// eg decaySeconds == 1 -> 1 / sampleRate()
		//    decaySeconds == 0.5 -> 1 / (0.5 * sampleRate), which is twice as fast, equivalent to 2 / sampleRate()
		// since we generate a new buffer every overlapSize samples, we multiply that rate by overlapSize, giving:
		const float decayDec = overlapSize / (decaySeconds * sampleRate());
		
        for( unsigned i = 0; i < specSize; ++i )
        {
            band& b = bands[i];
			b.decay = b.decay > decayDec ? b.decay - decayDec : 0;
			b.phase += b.phaseStep;
			
			const float x = b.decay*b.amplitude*cosf(bands[i].phase);
			const float y = b.decay*b.amplitude*sinf(bands[i].phase);
			
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
