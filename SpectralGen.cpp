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
#include <random>

SpectralGen::SpectralGen()
: UGen()
, decay( *this, CONTROL, 0 )
, brightness( *this, CONTROL, 0 )
, inverseSize(0)
, specSize(0)
, outputSize(0)
, outIndex(0)
, overlapSize(0)
, fft(nullptr)
, bands(nullptr)
, specReal(nullptr)
, specImag(nullptr)
, inverse(nullptr)
, output(nullptr)
, adjustOddPhase(false)
{
}

SpectralGen::~SpectralGen()
{
	cleanup();
}

void SpectralGen::reset()
{
	// pick a buffer size based on sample rate.
	// lower sample rate means shorter buffer, otherwise short decays will never be heard.
	const int bufferSizeNP2 = (int)(sampleRate()*0.09f);
	// find the nearest power of two greater than or equal to bufferSizeNP2
	// so that our spectrum and overlap behave nicely.
	int bufferSize = 32;
	while (bufferSize < bufferSizeNP2) bufferSize <<= 1;

	if (inverseSize != bufferSize)
	{
		cleanup();

		inverseSize = bufferSize;
		specSize = inverseSize / 2;
		outputSize = inverseSize * 2;
		overlapSize = inverseSize / 2;

		fft = new Minim::FFT(inverseSize, sampleRate());
		bands = new band[specSize];
		inverse = new float[inverseSize];
		specReal = new float[inverseSize];
		specImag = new float[inverseSize];
		output = new float[outputSize];
	}

	// have to set the sample rate always 
	// because we may not have resized the buffers even though the sample rate changed.
	fft->setSampleRate(sampleRate());

	memset(specReal, 0, sizeof(float)*inverseSize);
	memset(specImag, 0, sizeof(float)*inverseSize);
	memset(inverse, 0, sizeof(float)*inverseSize);
	memset(output, 0, sizeof(float)*outputSize);
	
	// we give each band a random phase to minimize interference
	// when there are lots of active bands.
	// ie this sounds nicer than if all bands start with a phase of 0.
	std::random_device rd;  //Will be used to obtain a seed for the random number engine
	std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
	std::uniform_real_distribution<> dis(0, M_PI*2);
	
	for(int i = 0; i < specSize; ++i)
	{
		bands[i].amplitude = 0;
		bands[i].phase = dis(gen);
	}
	
	outIndex = 0;
	adjustOddPhase = false;
	// spectral magnitude is relative to the fft size because shorter ffts make louder output (and vice-versa),
	// so this helps maintain similar volume across all sample rates.
#if SA_API
	// spectral magnitude needs to be louder for standalone to balance this APP_MULT constant in app_resource.h
	// adjust volume here gives better results than setting APP_MULT to 1 and using the same spectral amplitude.
	spectralMagnitude = inverseSize / 8;
#else
	spectralMagnitude = inverseSize / 32;
#endif
}

void SpectralGen::sampleRateChanged()
{
	reset();
}

void SpectralGen::pluck(const float freq, const float amp, const float spread)
{
	const float hs = spread / 2;
	const int cb = freqToIndex(freq);
	const int lb = freqToIndex(freq - hs);
	const int hb = freqToIndex(freq + hs);
	const int denom = cb - lb;
	for (int b = lb; b <= hb; ++b)
	{
		if (b >= 0 && b < specSize)
		{
			if (b == cb)
			{
				bands[b].amplitude = amp;
			}
			else
			{
				const float fn = fabs(b - cb) / denom;
				// exponential fall off from the center, see: https://www.desmos.com/calculator/gzqrz4isyb
				const float fa = amp * exp(-10 * fn);
				// we set the amplitude to the maximum of the current effective amplitude and the amplitude from the spread,
				// ie we don't want to make a band suddenly quieter than it currently is.
				bands[b].amplitude = fmax(bands[b].amplitude * bands[b].decay, fa);
			}

			bands[b].decay = 1;
		}
	}
}

float SpectralGen::getBandPhase(const float freq) const
{
	if (fft != nullptr)
	{
		const int b = fft->freqToIndex(freq);
		return b >= 0 && b < specSize ? bands[b].phase : 0;
	}

	return 0;
}

float SpectralGen::getBandMagnitude(const float freq) const
{
	if (fft != nullptr)
	{
		const int b = fft->freqToIndex(freq);
		return b >= 0 && b < specSize ? bands[b].amplitude*bands[b].decay : 0;
	}

	return 0;
}

void SpectralGen::uGenerate(float* out, const int numChannels)
{
	if (output == nullptr)
	{
		UGen::fill(out, 0, numChannels);
		return;
	}

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

		const float falloff = brightness.getLastValue();

		memset(specReal, 0, specSize * sizeof(float));
		memset(specImag, 0, specSize * sizeof(float));
		
        for( unsigned i = 0; i < specSize; ++i )
        {
            band& b = bands[i];
			b.decay = b.decay > decayDec ? b.decay - decayDec : 0;
			if (b.decay > 0)
			{
				// ODD bands need to step phase by 90 degrees every other generation
				// because our overlap is half the size of the buffer generated.
				// this ensure that phase lines up for those sinusoids in every buffer.
				float p = adjustOddPhase && i % 2 == 1 ? b.phase + M_PI : b.phase;
				float a = spectralMagnitude*b.decay*b.amplitude;

				addSinusoid(i, a, p);

				const float bandFreq = fft->indexToFreq(i);
				int partial = 2;
				int bidx = freqToIndex(bandFreq*partial);
				while (bidx < specSize)
				{
					p = adjustOddPhase && bidx % 2 == 1 ? b.phase + M_PI : b.phase;
					a *= falloff;
					addSinusoid(bidx, a / partial, p);
					++partial;
					bidx = freqToIndex(bandFreq*partial);
				}
			}
        }

        fft->Minim::FourierTransform::inverse(specReal, specImag, inverse);
        Minim::FourierTransform::TRIANGULAR.apply( inverse, inverseSize );
		
        for( int s = 0; s < inverseSize; ++s )
        {
            int ind = (s + outIndex) % outputSize;
            
            output[ind] += inverse[s];
        }
		
		adjustOddPhase = !adjustOddPhase;
    }
    
    UGen::fill(out, output[outIndex], numChannels);
    output[outIndex] = 0;
    
    outIndex = (outIndex+1)%outputSize;
}

int SpectralGen::freqToIndex(const float freq)
{
	// special case: freq is lower than the bandwidth of spectrum[0] but not negative
	if (freq > 0 && freq < fft->getBandWidth() / 2) return 0;
	// all other cases
	float fraction = freq / sampleRate();
	// roundf is not available in windows, so we do this
	int i = (int)floorf((float)fft->timeSize() * fraction + 0.5f);
	return i;
}

void SpectralGen::cleanup()
{
	if (fft != nullptr)
	{
		delete fft;
		fft = nullptr;
	}

	if (bands != nullptr)
	{
		delete[] bands;
		bands = nullptr;
	}

	if (specReal != nullptr)
	{
		delete[] specReal;
		specReal = nullptr;
	}

	if (specImag != nullptr)
	{
		delete[] specImag;
		specImag = nullptr;
	}

	if (inverse != nullptr)
	{
		delete[] inverse;
		inverse = nullptr;
	}

	if (output != nullptr)
	{
		delete[] output;
		output = nullptr;
	}
}