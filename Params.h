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
	kBandFirst,
	kBandLast,
	kBandDensity,
	kNumParams
};

enum ESettings
{
	// settings for the SpectralGen UGen
	kSpectralAmpMax = 1,
	kSpectralGenSize = 1024*8,

	// settings for kPitch, expressed as percent
	kPitchMin = 25,
	kPitchMax = 100,
	kPitchDefault = kPitchMax,

	// settings for kDecay, expressed as percent
	kDecayMin = 0,
	kDecayMax = 100,
	kDecayDefault = 50,

	// settings for kCrush, expressed as percent
	kCrushMin = 0,
	kCrushMax = 100,
	kCrushDefault = kCrushMin,

	// settings for kBandFirst and kBandLast params
	kBandMin = 12,
	kBandMax = (int)(kSpectralGenSize * 0.3),
	kBandMinDistance = 60,

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
