#pragma once

#include "IPlug_include_in_plug_hdr.h"

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

	void OnReset() override;
	void OnParamChange(int paramIdx) override;
	void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;
	
	int UnserializeState(const IByteChunk& chunk, int startPos) override;

	void BeginMIDILearn(int controlId, int paramIdx1, int paramIdx2, float x, float y);
	void ProcessMidiMsg(const IMidiMsg& msg) override;

	// catch the About menu item to display what we wants in a box
	bool OnHostRequestingAboutBox() override;
  // catch the Read Manual menu item to open the Manual
  bool OnHostRequestingProductHelp() override;

	void BroadcastParamChange(const int paramIdx);

	// given a string number between 0 and the current value of Density,
	// return the current frequency based on related parameters like BandFirst and BandLast.
	float FrequencyOfString(int stringNum);

	// can be called directly from StringControl, but also used internally in response to param changes and midi.
	void Pluck(const float pluckX, const float pluckY);	

private:

	void InitBandParam(const char * name, const int paramIdx, const int defaultValue);
	void HandleMidiControlChange(const IMidiMsg& msg);	
	void SetControlChangeForParam(const IMidiMsg::EControlChangeMsg cc, const int paramIdx);
	float GetPluckAmp(const float pluckY) const;
	void PluckSpectrum(const float freq, float mag);

  WDL_String      mIniPath;
	bool 					  mIsLoading;

	float					  mGain;
	float					  mPluckX;
	float					  mPluckY;
	float					  mSpread;
	float					  mBrightness;
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
