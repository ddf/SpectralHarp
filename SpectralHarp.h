#ifndef __SPECTRALHARP__
#define __SPECTRALHARP__

#include "IPlug_include_in_plug_hdr.h"
#include "src/SpectralGen.h"
#include "BitCrush.h"
#include "TickRate.h"
#include "MoogFilter.h"

const float kMaxSpectralAmp = 128.0f;

class SpectralHarp : public IPlug
{
public:
	SpectralHarp(IPlugInstanceInfo instanceInfo);
	~SpectralHarp();

	void Reset();
	void OnParamChange(int paramIdx);
	void ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames);

private:

	void pluck();

	float					  mGain;
	float					  mPluckX;
	float					  mPluckY;
	SpectralGen               specGen;
	Minim::BitCrush           bitCrush;
	Minim::TickRate           tickRate;
	Minim::MoogFilter         highPass;
};

#endif
