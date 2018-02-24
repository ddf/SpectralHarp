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
	kBandMinDistance = 256,

	kBandFirstDefault = kBandMin,
	kBandLastDefault = kBandMax,
	
	// settings for kBandDensity
	kBandDensityMin = 16,
	kBandDensityMax = 256,
	kBandDensityDefault = kBandDensityMax
};

static float Map(float value, float istart, float istop, float ostart, float ostop)
{
	return ostart + (ostop - ostart) * ((value - istart) / (istop - istart));
}
