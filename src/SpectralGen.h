//
//  SpectralGen.h
//  SpectralHarp
//
//  Created by Damien Di Fede on 5/6/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef SpectralHarp_SpectralGen_h
#define SpectralHarp_SpectralGen_h

#include "UGen.h"
#include "FFT.h"

class SpectralGen : public Minim::UGen
{
public:
    SpectralGen( const int timeSize );
    virtual ~SpectralGen();
    
    void    setBand( const int b, const float mag, const float phase )
    {
        amplitudes[b] = mag;
        phases[b]     = phase;
    }
    
    void    setBandMagnitude( const int b, const float mag ) { amplitudes[b] = mag; }
    float   getBandMagnitude( const int b ) const { return amplitudes[b]; }
    
    void    setBandPhase( const int b, const float phase ) { phases[b] = phase; }
    float   getBandPhase( const int b ) const { return phases[b]; }
    
    void    setBandPhaseStep( const int b, const float phaseStep ) { phaseSteps[b] = phaseStep; }
    
protected:
    
    virtual void uGenerate( float* out, const int numChannels );
    
private:
    
    // used for synthesis
    Minim::FFT  fft;
    
    // spectrum information
    float*  specReal;
    float*  specImag;
    
    // amplitudes and phases of bands in the spectrum
    float*  amplitudes;
    float*  phases;
    float*  phaseSteps;
    
    int     timeSize;
    int     specSize;
    int     windowSize;
    int     outIndex;
    float*  output;
    float*  inverse;
};

#endif
