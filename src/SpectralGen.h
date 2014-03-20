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

const int kSpectralGenSize  = 1024 * 8;

class SpectralGen : public Minim::UGen
{
public:
    SpectralGen( const int timeSize = kSpectralGenSize );
    virtual ~SpectralGen();
    
    inline void  pluck( const int b, const float amp, const float ps ) { bands[b].pluck(amp,ps); }
    
    inline float getBandMagnitude( const int b ) const { return bands[b].amplitude; }
    inline float getBandPhase( const int b ) const { return bands[b].phase; }
    
    static float decay;
    
protected:
    
    virtual void uGenerate( float* out, const int numChannels );
    
private:
    
    struct band
    {
        // amplitude applied on strum
        float struckAmplitude;
        // current amplitude
        float amplitude;
        // current phase
        float phase;
        // how much to step the phase on update
        float phaseStep;
        
        band()
        : struckAmplitude(0)
        , amplitude(0)
        , phase(0)
        , phaseStep(0)
        {

        }
        
        inline void pluck( float amp, float ps )
        {
            amplitude = struckAmplitude = amp;
            phaseStep = ps;
        }
        
        inline void update()
        {
            const float d     = struckAmplitude*decay;
            amplitude         = amplitude-d > 0 ? amplitude-d : 0;
            phase             = phase + phaseStep > M_2_PI ? phase - M_2_PI + phaseStep : phase + phaseStep;
        }
        
    };
    
    band bands[kSpectralGenSize/2];
    
    // used for synthesis
    Minim::FFT  fft;
    
    // spectrum information
    float*  specReal;
    float*  specImag;
    
    int     timeSize;
    int     specSize;
    int     windowSize;
    int     outIndex;
    float*  output;
    float*  inverse;
};

#endif
