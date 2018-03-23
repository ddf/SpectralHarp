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
    SpectralGen();
    virtual ~SpectralGen();
	
	void reset();
	
	void  pluck( const float freq, const float amp, const float spread );
	
	float getBandMagnitude( const float freq ) const;
	float getBandPhase( const float freq ) const;
    
	UGenInput decay;
    
protected:

	// our own version of this so we can return out-of-bounds indices.
	// helpful when dealing with large spread near the edge of the spectrum.
	int freqToIndex(const float freq);
    
	void cleanup();
    void uGenerate( float* out, const int numChannels ) override;
	void sampleRateChanged() override;
    
private:
    
    struct band
    {
        // the amplitude of the band
        float amplitude;
		// current decay of the band (0-1), scales amplitude
		float decay;
		float phase;
        
        band()
        : amplitude(0)
		, decay(0)
		, phase(0)
        {

        }
    };
    
	band* bands;
	
    // used for synthesis
    Minim::FFT*  fft;
    
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
	// whether we should adjust the phase of odd bands in the next buffer generation step
	bool 	adjustOddPhase;
	// the fixed magnitude we use to scale up bands before performing the inverse
	int    spectralMagnitude;
};

#endif
