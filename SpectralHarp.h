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

	void ProcessMidiMsg(const IMidiMsg& msg) override;

	// catch the About menu item to display what we wants in a box
	bool OnHostRequestingAboutBox() override;
  // catch the Read Manual menu item to open the Manual
  bool OnHostRequestingProductHelp() override;

  // update the UI with current midi mappings
  void OnUIOpen() override;

  // handle messages from the UI
  bool OnMessage(int messageTag, int controlTag, int dataSize, const void* pData) override;

#if APP_API
	void BroadcastParamChange(const int paramIdx);
#endif

	// given a string number between 0 and the current value of Density,
	// return the current frequency based on related parameters like BandFirst and BandLast.
	float FrequencyOfString(int stringNum);		  

private:

	void InitBandParam(const char * name, const int paramIdx, const int defaultValue);

	float GetPluckAmp(const float pluckY) const;
  // figure out where to PluckSpectrum based on normalized xy coordinates
  void Pluck(const float pluckX, const float pluckY);
	void PluckSpectrum(const float freq, float mag);

#if APP_API
  void HandleMidiControlChange(const IMidiMsg& msg);
  void SetControlChangeForParam(const IMidiMsg::EControlChangeMsg cc, const int paramIdx);
#endif

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

#if APP_API  
	// for each param, which midi control change should set its value
	IMidiMsg::EControlChangeMsg controlChangeForParam[kNumParams];
#endif

	IMidiQueue				  mMidiQueue;
	std::vector<IMidiMsg>	  mNotes;	
};
