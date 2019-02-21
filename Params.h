//
//  Params.h
//  SpectralHarp
//
//  Created by Damien Quartz on 2/20/18
//

#pragma once

#include "Frequency.h"

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
	// how "bright" we make the sound by exciting additional partials of a string.
	// at 100% a single string will sound like a band-limited saw wave instead of a sine wave.
	kBrightness,
	kNumParams
};

// used by the UI to send messages to the main plugin class.
enum EMessages
{
  kSetMidiMapping,
  kPluckSpectrum,
};

// data payload for the SetMidiMapping message
struct MidiMapping
{
  enum CC
  {
    kNone = 128
  };

  const int param;
  const CC midiCC;

  MidiMapping(int p, CC cc = kNone) : param(p), midiCC(cc) {}
};

struct PluckCoords
{
  const float x, y;

  PluckCoords(float _x, float _y) : x(_x), y(_y) {}
};

// control tags
enum ETags
{
  kMidiMapper,
  kStringControl,
};

enum ESettings
{
  // settings for the SpectralGen UGen
  kSpectralAmpMax = 1,
  kSpectralSizeMax = 1<<18,

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

  kUmappedParamCC = 128 // MIDI CC value we use as a stand-in for a parameter that does not have a midi mapping
};

static float Map(float value, float istart, float istop, float ostart, float ostop)
{
	return ostart + (ostop - ostart) * ((value - istart) / (istop - istart));
}

static float FrequencyOfString(int stringNum, float stringCount, float lowFreqHz, float hiFreqHz, float linLogLerp)
{
  const float t = (float)stringNum / stringCount;
  // convert first and last bands to midi notes and then do a linear interp, converting back to Hz at the end.
  Minim::Frequency lowFreq = Minim::Frequency::ofHertz(lowFreqHz);
  Minim::Frequency hiFreq = Minim::Frequency::ofHertz(hiFreqHz);
  const float linFreq = Map(t, 0, 1, lowFreq.asHz(), hiFreq.asHz());
  const float midiNote = Map(t, 0, 1, lowFreq.asMidiNote(), hiFreq.asMidiNote());
  const float logFreq = Minim::Frequency::ofMidiNote(midiNote).asHz();
  // we lerp from logFreq up to linFreq because log spacing clusters frequencies
  // towards the bottom of the range, which means that when holding down the mouse on a string
  // and lowering this param, you'll hear the pitch drop, which makes more sense than vice-versa.
  return Map(linLogLerp, 0, 1, logFreq, linFreq);
}
