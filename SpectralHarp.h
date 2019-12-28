#pragma once

#include "IPlug_include_in_plug_hdr.h"

#include "Params.h"
#include "Smoothers.h"

#include "SpectralGen.h"
#include "BitCrush.h"
#include "TickRate.h"
#include "StringControl.h"

#include <vector>

using namespace iplug;
using namespace igraphics;

class PLUG_CLASS_NAME : public Plugin
{
public:
	PLUG_CLASS_NAME(const InstanceInfo& instanceInfo);

#if IPLUG_DSP
	void OnReset() override;
	void OnParamChange(int paramIdx) override;
	void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;
  void ProcessMidiMsg(const IMidiMsg& msg) override;

  // handle messages from the UI
  bool OnMessage(int messageTag, int controlTag, int dataSize, const void* pData) override;
  // transmit data to the UI
  void OnIdle() override;
#endif
	
	int UnserializeState(const IByteChunk& chunk, int startPos) override;	

	// catch the About menu item to display what we wants in a box
	bool OnHostRequestingAboutBox() override;
  // catch the Read Manual menu item to open the Manual
  bool OnHostRequestingProductHelp() override;

  // update the UI with current midi mappings
  void OnUIOpen() override;

#if APP_API
	void BroadcastParamChange(const int paramIdx);
#endif

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

#if IPLUG_DSP
  float           mBandFirst;
  float           mBandLast;
	float					  mGain;
	float					  mPluckX;
	float					  mPluckY;
	float					  mSpread;
	float					  mBrightness;
  LogParamSmooth<float>  mShift;
	SpectralGen               specGen;
	Minim::BitCrush           bitCrush;
	Minim::TickRate           tickRate;

  IMidiQueue				  mMidiQueue;
  std::vector<IMidiMsg>	  mNotes;
  StringControl::Capture    spectrumCapture;
#endif

#if APP_API  
	// for each param, which midi control change should set its value
	IMidiMsg::EControlChangeMsg controlChangeForParam[kNumParams];
#endif		
};
