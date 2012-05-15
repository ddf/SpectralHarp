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
    
    amplitudes = new float[specSize];
    memset( amplitudes, 0, sizeof(float)*specSize );
    phases     = new float[specSize];
    memset( phases, 0, sizeof(float)*specSize );
    phaseSteps = new float[specSize];
    memset( phaseSteps, 0, sizeof(float)*specSize );
}

SpectralGen::~SpectralGen()
{
    delete [] specReal;
    delete [] specImag;
    delete [] inverse;
    delete [] output;
    delete [] amplitudes;
    delete [] phases;
    delete [] phaseSteps;
}

void SpectralGen::uGenerate(float* out, const int numChannels)
{
    if ( outIndex % windowSize == 0 )
    {
        for( int b = 0; b < specSize; ++b )
        {
            float amp   = amplitudes[b];
            float phase = phases[b];
            specReal[b] = amp*cosf(phase);
            specImag[b] = amp*sinf(phase);
            
            if ( b > 0 && b < specSize )
            {
                specReal[timeSize - b] = specReal[b];
                specImag[timeSize - b] = -specImag[b];
            }
            
            //phases[b] += phaseSteps[b];
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