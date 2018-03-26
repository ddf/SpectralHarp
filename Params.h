//
//  Params.h
//  SpectralHarp
//
//  Created by Damien Quartz on 2/20/18
//

#pragma once

enum EParams
{
	kVolume = 0,
	kPitch,
	kDecay,
	kCrush,
	// params for the xy pad that can be used to "strum"
	kPluckX,
	kPluckY,
	// frequency of the first strummable string
	kBandFirst,
	// frequency of the last strummable string
	kBandLast,
	// how many strings available for strumming
	kBandDensity,
	// at 0 we space strings linearly between BandFirst and BandLast,
	// at 1 we space strings logarithmically between BandFirst and BandLast,
	// at values inbetween we lerp between the linear value and log value.
	kBandLinLogLerp,
	// spread is expressed in Hz and is how widely we excite the spectrum around a strings frequency.
	// applied amplitudes fall of exponentially a we move away from the central frequency band.
	// how much of an audible effect this will have depends on the sample rate, 
	// since higher sample rates means we get more resolution in the frequency spectrum.
	kBandSpread,
	kNumParams
};

enum ESettings
{
	// settings for the SpectralGen UGen
	kSpectralAmpMax = 1,

	// settings for kPitch, expressed as percent
	kPitchMin = 50,
	kPitchMax = 100,
	kPitchDefault = kPitchMax,

	// settings for kDecay, expressed in milliseconds
	kDecayMin = 150,
	kDecayMax = 5000,
	kDecayDefault = 500,

	// settings for kCrush, expressed as percent
	kCrushMin = 0,
	kCrushMax = 100,
	kCrushDefault = kCrushMin,

	// settings for kBandFirst and kBandLast params
	kBandMin = 64,
	kBandMax = 13000,
	kBandMinDistance = 64,

	kBandFirstDefault = kBandMin,
	kBandLastDefault = kBandMax,
	
	// settings for kBandDensity
	kBandDensityMinOrig = 16,
	kBandDensityMaxOrig = 256,
	kBandDensityMin = 12, // one string per semi-tone if low and high frequency are an octave apart
	kBandDensityMax = 288, // 24 "octaves" of semi-tones
	kBandDensityDefault = kBandDensityMax,

	kBandSpreadMin = 0,
	kBandSpreadMax = 13000,
};

static float Map(float value, float istart, float istop, float ostart, float ostop)
{
	return ostart + (ostop - ostart) * ((value - istart) / (istop - istart));
}
