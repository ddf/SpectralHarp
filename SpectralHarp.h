#ifndef __SPECTRALHARP__
#define __SPECTRALHARP__

#include "IPlug_include_in_plug_hdr.h"
#include "IMidiQueue.h"

#include "Params.h"

#include "SpectralGen.h"
#include "BitCrush.h"
#include "TickRate.h"
#include "MoogFilter.h"

class SpectralHarp : public IPlug
{
public:
	SpectralHarp(IPlugInstanceInfo instanceInfo);
	~SpectralHarp();

	void Reset() override;
	void OnParamChange(int paramIdx) override;
	void ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames) override;

	void BeginMIDILearn(int paramIdx1, int paramIdx2, int x, int y);
	virtual void ProcessMidiMsg(IMidiMsg* pMsg) override;

private:

	void InitBandParam(const char * name, const int paramIdx, const int defaultValue);
	void HandleMidiControlChange(IMidiMsg* pMsg);
	void Pluck();

	float					  mGain;
	float					  mPluckX;
	float					  mPluckY;
	SpectralGen               specGen;
	Minim::BitCrush           bitCrush;
	Minim::TickRate           tickRate;
	Minim::MoogFilter         highPass;

	// if not -1 when we receive a control change midi message
	// we use this to determine which param should be linked to the control change
	int						  midiLearnParamIdx;
	// for each param, which midi control change should set its value
	IMidiMsg::EControlChangeMsg controlChangeForParam[kNumParams];

	IMidiQueue				  mMidiQueue;
};

#endif
