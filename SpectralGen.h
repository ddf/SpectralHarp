//
//  SpectralGen.h
//  SpectralHarp
//
//  Created by Damien Di Fede on 5/6/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef SpectralHarp_SpectralGen_h
#define SpectralHarp_SpectralGen_h

#include "Params.h"
#include "UGen.h"
#include "FFT.h"

class SpectralGen : public Minim::UGen
{
public:
    SpectralGen( const int timeSize = kSpectralGenSize );
    virtual ~SpectralGen();
	
	void reset();
	
	void  pluck( const int b, const float amp );
    
    inline float getBandMagnitude( const int b ) const { return bands[b].amplitude; }
	inline float getBandPhase( const int b ) const { return bands[b].phase; }
	inline float getBandFrequency(const int b) const { return fft.indexToFreq(b); }
    
	UGenInput decayRate;
    
protected:
    
    void uGenerate( float* out, const int numChannels ) override;
	void sampleRateChanged() override;
    
private:
    
    struct band
    {
        // amplitude applied on strum
        float struckAmplitude;
        // current amplitude
        float amplitude;
		float phase;
		float phaseStep;
        
        band()
        : struckAmplitude(0)
        , amplitude(0)
		, phase(0)
		, phaseStep(0)
        {

        }
        
        inline void pluck( const float amp )
        {
            amplitude = struckAmplitude = amp;
        }
        
        inline void update( const float decay )
        {
            const float d     = struckAmplitude*decay;
            amplitude         = amplitude-d > 0 ? amplitude-d : 0;
			phase += phaseStep;
        }
        
    };
    
    band bands[kSpectralGenSize/2];
	
    // used for synthesis
    Minim::FFT  fft;
    
    // spectrum information
    float*  specReal;
    float*  specImag;
	
	// buffer we write into when performing the ifft
	float*  inverse;
	// circular output buffer that we add into after synthesizing into inverse
	float*  output;
	// where we should next read from output
	int     outIndex;
	
	// how big is the output buffer
	int     outputSize;
	// how big is the inverse buffer (specReal and specImag are also this size)
	int     inverseSize;
	// how many "positive" frequency bands in the fft (always timeSize/2)
    int     specSize;
	// how many samples we overlap our generated samples
	// (ie every overlapSize samples we update our plucked bands and synthesize into inverse)
    int     overlapSize;
};

#endif
