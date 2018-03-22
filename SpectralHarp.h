#ifndef __SPECTRALHARP__
#define __SPECTRALHARP__

#include "IPlug_include_in_plug_hdr.h"
#include "IMidiQueue.h"

#include "Params.h"

#include "SpectralGen.h"
#include "BitCrush.h"
#include "TickRate.h"
#include <vector>

class SpectralHarp : public IPlug
{
public:
	SpectralHarp(IPlugInstanceInfo instanceInfo);
	~SpectralHarp();

	void Reset() override;
	void OnParamChange(int paramIdx) override;
	void ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames) override;
	
	int UnserializeState(ByteChunk* pChunk, int startPos) override;

	void BeginMIDILearn(int paramIdx1, int paramIdx2, int x, int y);
	void ProcessMidiMsg(IMidiMsg* pMsg) override;

	// catch the About menu item to display what we wants in a box
	bool HostRequestingAboutBox() override;

	void BroadcastParamChange(const int paramIdx);
	// can be called directly from StringControl, but also used internally in response to param changes and midi.
	void Pluck(const float pluckX, const float pluckY);

private:

	void InitBandParam(const char * name, const int paramIdx, const int defaultValue);
	void HandleMidiControlChange(IMidiMsg* pMsg);	
	void SetControlChangeForParam(const IMidiMsg::EControlChangeMsg cc, const int paramIdx);
	
	bool 					  mIsLoading;

	float					  mGain;
	float					  mPluckX;
	float					  mPluckY;
	SpectralGen               specGen;
	Minim::BitCrush           bitCrush;
	Minim::TickRate           tickRate;

	// if not -1 when we receive a control change midi message
	// we use this to determine which param should be linked to the control change
	int						  midiLearnParamIdx;
	// for each param, which midi control change should set its value
	IMidiMsg::EControlChangeMsg controlChangeForParam[kNumParams];

	IMidiQueue				  mMidiQueue;
	std::vector<IMidiMsg>	  mNotes;
};

#endif
