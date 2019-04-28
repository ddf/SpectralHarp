#include "SpectralHarp.h"
#include "IPlug_include_in_plug_src.h"
#include "IPlugPaths.h"
#include "IControl.h"
#include "resource.h"

#include "MidiMapper.h"
#include "StringControl.h"
#include "SpectrumSelection.h"
#include "KnobLineCoronaControl.h"
#include "TextBox.h"

#if APP_API
static const char * kAboutBoxText = "Version " PLUG_VERSION_STR "\nCreated by Damien Quartz\nBuilt on " __DATE__;

#ifdef OS_MAC
#include "swell.h"
#endif

// name of the section of the INI file we save midi cc mappings to
const char * kMidiControlIni = "midi";
const IMidiMsg::EControlChangeMsg kUnmappedParam = (IMidiMsg::EControlChangeMsg)MidiMapping::kNone;

#endif

const int kNumPrograms = 1;

#if IPLUG_EDITOR
enum ELayout
{
	kWidth = PLUG_WIDTH,
	kHeight = PLUG_HEIGHT,

	kControlPanelHeight = 120,

	kPluckPadHeight = kHeight - kControlPanelHeight,
	kPluckPadMargin = 0,
	kPluckPadSpaceBottom = 5,

	kSpectrumSelect_W = 830,
	kSpectrumSelect_X = kWidth / 2 - kSpectrumSelect_W / 2,
	kSpectrumSelect_Y = kPluckPadHeight + kPluckPadSpaceBottom,
	kSpectrumSelect_H = 16,

	kKnob_X = 0,
	kKnob_Y = kSpectrumSelect_Y + kSpectrumSelect_H + 30,
	kKnob_W = 48,
	kKnob_H = 48,

	kKnobLineThickness = 1,
	kKnobSpacing = 77,

	kVolumeX = PLUG_WIDTH - kKnob_W - 14,

	kSpreadX = kWidth/2 - kKnob_W/2,

	kBandDensityX = kSpreadX - kKnobSpacing*3,
	kTuningX      = kSpreadX - kKnobSpacing*2,
	kDecayX       = kSpreadX - kKnobSpacing,
	
	kBrightnessX  = kSpreadX + kKnobSpacing,
	kPitchX       = kSpreadX + kKnobSpacing*2,
	kCrushX       = kSpreadX + kKnobSpacing*3,

	kKnobFrames = 60,

	kCaptionT = kKnob_Y + 45,
	kCaptionB = kCaptionT + 15,
	kCaptionW = 50,

	kTextBoxW = 64,
	kTextBoxH = kSpectrumSelect_H + 2,
	
	kTitleX = 10,
	kTitleBottomMargin = 10
};

// background of entire window
static const IColor backColor = IColor(255, 15, 15, 15);
// rectangular panel behind the knobs
static const IColor panelColor = IColor(255, 30, 30, 30);
// color of shadow cast on strings by the knob panel
static const IColor shadowColor = IColor(200, 10, 10, 10);
static const char * interfaceFontId = "interfaceFont";
static const char * interfaceFontName = ROBOTTO_FN;
// text color for labels under the knobs
static const IColor labelColor = IColor(255, 200, 200, 200);
static const int    labelSize  = 12;
// text style
static const IColor titleColor = IColor(255, 60, 60, 60);
static const char * titleString = PLUG_NAME " " PLUG_VERSION_STR;
static const int titleSize = 16;

// spectrum selection colors
static const IColor selectionBackColor = backColor;
static const IColor selectionSelectColor = IColor(255, 80, 80, 80);
static const IColor selectionHandleColor = IColor(255, 200, 200, 200);

// knob colors
static const IColor knobColor = IColor(255, 200, 200, 200);
#endif

#if IPLUG_DSP // used in ProcessBlock
float expoEaseOut(float t, float b, float c, float d)
{
	return c * (-powf(2, -10 * t / d) + 1) + b;
}
#endif

PLUG_CLASS_NAME::PLUG_CLASS_NAME(IPlugInstanceInfo instanceInfo)
	: IPLUG_CTOR(kNumParams, kNumPrograms, instanceInfo)
	, mIsLoading(false)
#if IPLUG_DSP
	, mGain(1.)
	, mPluckX(0)
	, mPluckY(0)
	, mSpread(0)
	, mBrightness(0)
	, specGen()
	, bitCrush(24, 44100)
	, tickRate(1)
#endif
{
	TRACE;

#if APP_API
	for (int i = 0; i < kNumParams; ++i)
	{
		controlChangeForParam[i] = kUnmappedParam;
	}
#endif

	//arguments are: name, defaultVal, minVal, maxVal, step, label, flags, group, shape.
  // if no shape is provided, then it will be linearaly shaped
	GetParam(kVolume)->InitDouble("Volume", 25.0, 0.0, 100.0, 0.1, "%", 0, "", IParam::ShapePowCurve(2.));
	GetParam(kPitch)->InitDouble("Pitch", kPitchDefault, kPitchMin, kPitchMax, 0.1, "%");
	GetParam(kDecay)->InitInt("Decay", kDecayDefault, kDecayMin, kDecayMax, "ms", 0);
	GetParam(kCrush)->InitDouble("Crush", kCrushDefault, kCrushMin, kCrushMax, 0.1, "%");
	GetParam(kPluckX)->InitDouble("Pluck X", 0, 0., 100.0, 0.1, "%");
	GetParam(kPluckY)->InitDouble("Pluck Y", 100.0, 0., 100.0, 0.1, "%");

	InitBandParam("First Band", kBandFirst, kBandFirstDefault);
	InitBandParam("Last Band", kBandLast, kBandLastDefault);

	GetParam(kBandDensity)->InitInt("Density", kBandDensityDefault, kBandDensityMin, kBandDensityMax, "strings");
	// default of 1, which is linear spacing like in the first version, which means the sound is preserved for existing projects.
	GetParam(kBandLinLogLerp)->InitDouble("Tuning", 1, 0, 1, 0.01, "log -> lin");

	// default of 0, which matches behavior of the first version.
	GetParam(kBandSpread)->InitDouble("Spread", kBandSpreadMin, kBandSpreadMin, kBandSpreadMax, 1, "Hz");
	//GetParam(kBandSpread)->SetShape(2);

	GetParam(kBrightness)->InitDouble("Brightness", 0, 0, 100, 0.1, "%");

#if IPLUG_EDITOR
  mMakeGraphicsFunc = [&]() {
    return MakeGraphics(*this, kWidth, kHeight, PLUG_FPS);
  };

  mLayoutFunc = [&](IGraphics* pGraphics) {

    pGraphics->HandleMouseOver(true);
    pGraphics->AttachCornerResizer();
    pGraphics->AttachPanelBackground(backColor);
    // only use the midi mapper in the App because otherwise we leave mapping up to the DAW
#if APP_API
    pGraphics->AttachControl(new MidiMapper(), kMidiMapper);
#endif
    pGraphics->LoadFont(interfaceFontId, interfaceFontName);

    IText captionText = IText(labelSize, labelColor, interfaceFontId);
    captionText.mVAlign = IText::kVAlignTop;
    captionText.mTextEntryBGColor = backColor;
    captionText.mTextEntryFGColor = labelColor;

    IRECT strumRect = IRECT(kPluckPadMargin, 0, kWidth - kPluckPadMargin, kPluckPadHeight);
    pGraphics->AttachControl(new StringControl(strumRect, 10), kStringControl);

    IRECT stringShadowRect = IRECT(0, kPluckPadHeight - 5, kWidth, kPluckPadHeight);
    pGraphics->AttachControl(new IPanelControl(stringShadowRect, shadowColor));

    pGraphics->AttachControl(new IPanelControl(IRECT(0, kPluckPadHeight, kWidth, kHeight), panelColor));

    const int arrowMargin = 14;
    IRECT arrowRect = IRECT(strumRect.L + arrowMargin, strumRect.B, strumRect.R - arrowMargin, strumRect.B + kSpectrumSelect_H + kPluckPadSpaceBottom + 5);
    pGraphics->AttachControl(new SpectrumArrows(arrowRect, labelColor));

    // string Hz labels
    {
      IText bandLabel = captionText;
      bandLabel.mAlign = IText::kAlignCenter;
      bandLabel.mVAlign = IText::kVAlignMiddle;
      const int capMargin = 24;
      const int textBoxTop = kSpectrumSelect_Y;
      //bandLabel.mAlign = IText::kAlignNear;
      IRECT lowBandRect = IRECT(strumRect.L + capMargin, textBoxTop, strumRect.L + kTextBoxW + capMargin, textBoxTop + kTextBoxH);
      pGraphics->AttachControl(new TextBox(lowBandRect, kBandFirst, bandLabel, pGraphics, "00000 Hz", true, 0.005));

      //bandLabel.mAlign = IText::kAlignFar;
      IRECT highBandRect = IRECT(strumRect.R - kTextBoxW - capMargin, textBoxTop, strumRect.R - capMargin, textBoxTop + kTextBoxH);
      pGraphics->AttachControl(new TextBox(highBandRect, kBandLast, bandLabel, pGraphics, "00000 Hz", true, 0.005));
    }

    KnobLineCoronaControl* knob = new KnobLineCoronaControl(MakeIRectHOffset(kKnob, kVolumeX), kVolume, knobColor, knobColor, kKnobLineThickness);
    ITextControl* text = new ITextControl(IRECT(kVolumeX, kCaptionT, kVolumeX + kCaptionW, kCaptionB), "Volume", captionText);
    knob->SetLabelControl(text, "Volume");
    pGraphics->AttachControl(knob);
    pGraphics->AttachControl(text);

    // spectrum selection UI
    {
      IRECT rect = MakeIRect(kSpectrumSelect);
      SpectrumHandle* low = new SpectrumHandle(kBandFirst);
      SpectrumHandle* high = new SpectrumHandle(kBandLast);
      pGraphics->AttachControl(new SpectrumSelection(rect, low, high, selectionBackColor, selectionSelectColor, selectionHandleColor));
      pGraphics->AttachControl(low);
      pGraphics->AttachControl(high);

      rect.T += kSpectrumSelect_H + 3;
      rect.B += kSpectrumSelect_H + 3;
      pGraphics->AttachControl(new ITextControl(rect, "Spectrum Selection", captionText));
    }

    knob = new KnobLineCoronaControl(MakeIRectHOffset(kKnob, kBandDensityX), kBandDensity, knobColor, knobColor, kKnobLineThickness);
    text = new ITextControl(IRECT(kBandDensityX, kCaptionT, kBandDensityX + kCaptionW, kCaptionB), "Density", captionText);
    knob->SetLabelControl(text, "Density");
    pGraphics->AttachControl(knob);
    pGraphics->AttachControl(text);

    knob = new KnobLineCoronaControl(MakeIRectHOffset(kKnob, kTuningX), kBandLinLogLerp, knobColor, knobColor, kKnobLineThickness);
    text = new ITextControl(IRECT(kTuningX, kCaptionT, kTuningX + kCaptionW, kCaptionB), "Tuning", captionText);
    knob->SetLabelControl(text, "Tuning");
    pGraphics->AttachControl(knob);
    pGraphics->AttachControl(text);

    knob = new KnobLineCoronaControl(MakeIRectHOffset(kKnob, kSpreadX), kBandSpread, knobColor, knobColor, kKnobLineThickness);
    text = new ITextControl(IRECT(kSpreadX, kCaptionT, kSpreadX + kCaptionW, kCaptionB), "Spread", captionText);
    knob->SetLabelControl(text, "Spread");
    pGraphics->AttachControl(knob);
    pGraphics->AttachControl(text);

    knob = new KnobLineCoronaControl(MakeIRectHOffset(kKnob, kBrightnessX), kBrightness, knobColor, knobColor, kKnobLineThickness);
    text = new ITextControl(IRECT(kBrightnessX, kCaptionT, kBrightnessX + kCaptionW, kCaptionB), "Brightness", captionText);
    knob->SetLabelControl(text, "Brightness");
    pGraphics->AttachControl(knob);
    pGraphics->AttachControl(text);

    knob = new KnobLineCoronaControl(MakeIRectHOffset(kKnob, kPitchX), kPitch, knobColor, knobColor, kKnobLineThickness);
    text = new ITextControl(IRECT(kPitchX, kCaptionT, kPitchX + kCaptionW, kCaptionB), "Pitch", captionText);
    knob->SetLabelControl(text, "Pitch");
    pGraphics->AttachControl(knob);
    pGraphics->AttachControl(text);

    knob = new KnobLineCoronaControl(MakeIRectHOffset(kKnob, kDecayX), kDecay, knobColor, knobColor, kKnobLineThickness);
    text = new ITextControl(IRECT(kDecayX, kCaptionT, kDecayX + kCaptionW, kCaptionB), "Decay", captionText);
    knob->SetLabelControl(text, "Decay");
    pGraphics->AttachControl(knob);
    pGraphics->AttachControl(text);

    knob = new KnobLineCoronaControl(MakeIRectHOffset(kKnob, kCrushX), kCrush, knobColor, knobColor, kKnobLineThickness);
    text = new ITextControl(IRECT(kCrushX, kCaptionT, kCrushX + kCaptionW, kCaptionB), "Crush", captionText);
    knob->SetLabelControl(text, "Crush");
    pGraphics->AttachControl(knob);
    pGraphics->AttachControl(text);

    // logooo
    {
      IText titleText(titleSize, titleColor, interfaceFontId);
      titleText.mAlign = IText::kAlignNear;
      titleText.mVAlign = IText::kVAlignTop;
      IRECT titleRect(kTitleX, kCaptionT, kTitleX + 150, PLUG_HEIGHT);
      pGraphics->AttachControl(new ITextControl(titleRect, titleString, titleText));
    }
  };

  INIPath(mIniPath, BUNDLE_NAME);
  if (mIniPath.GetLength())
  {
    mIniPath.Append("\\settings.ini");
  }

#endif // IPLUG_EDITOR

  // do we stil need this?
	//MakeDefaultPreset((char *) "-", kNumPrograms);

	//-- AUDIO --------------------------------------
#if IPLUG_DSP
  mNotes.reserve(32);
	
  tickRate.value.setLastValue((float)GetParam(kPitch)->Value() / 100.0f);
  tickRate.setInterpolation(true);

  specGen.patch(tickRate).patch(bitCrush);
  bitCrush.setAudioChannelCount(1);
  bitCrush.setSampleRate(44100);
#endif
}

void PLUG_CLASS_NAME::InitBandParam(const char * name, const int paramIdx, const int defaultValue)
{
	IParam* param = GetParam(paramIdx);
	param->InitInt(name, defaultValue, kBandMin, kBandMax, "Hz");
}

void PLUG_CLASS_NAME::OnUIOpen()
{
  IPlug::OnUIOpen();

#if APP_API
  // read control mappings from the INI if we are running standalone
  for (int i = 0; i < kNumParams; ++i)
  {
    int defaultMapping = (int)controlChangeForParam[i];
    char controlName[32];
    sprintf(controlName, "control%u", i);
    controlChangeForParam[i] = (IMidiMsg::EControlChangeMsg)GetPrivateProfileInt(kMidiControlIni, controlName, defaultMapping, mIniPath.Get());

    MidiMapping map(i, (MidiMapping::CC)controlChangeForParam[i]);
    SendControlMsgFromDelegate(kMidiMapper, kSetMidiMapping, sizeof(MidiMapping), &map);

    BroadcastParamChange(i);
  }
#endif
}

int PLUG_CLASS_NAME::UnserializeState(const IByteChunk& chunk, int startPos)
{
  TRACE;
  //IMutexLock lock(this);

  mIsLoading = true;
  int endPos = UnserializeParams(chunk, startPos);
  mIsLoading = false;
  return endPos;
}

#if IPLUG_DSP
void PLUG_CLASS_NAME::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
	// Mutex is already locked for us.
	const double crushBegin = GetSampleRate();
	const double crushChange = 1000 - crushBegin;

  sample* in1 = inputs[0];
  sample* in2 = inputs[1];
  sample* out1 = outputs[0];
  sample* out2 = outputs[1];

	// excite all of the held frequencies
	for (auto& note : mNotes)
	{
		const Minim::Frequency freq = Minim::Frequency::ofMidiNote(note.NoteNumber());
		const float amp = GetPluckAmp((float)note.Velocity() / 127.0f);
		PluckSpectrum(freq.asHz(), amp);
	}

  bool processSpectrum = false;
	float result[1];
	for (int s = 0; s < nFrames; ++s, ++in1, ++in2, ++out1, ++out2)
	{
		while (!mMidiQueue.Empty())
		{
			IMidiMsg& pMsg = mMidiQueue.Peek();
			if (pMsg.mOffset > s) break;

			switch (pMsg.StatusMsg())
			{
#if APP_API
			case IMidiMsg::kControlChange:
				HandleMidiControlChange(pMsg);
				break;
#endif

			case IMidiMsg::kNoteOn:
				if (pMsg.Velocity() > 0)
				{
					// pluck the string now
					const Minim::Frequency freq = Minim::Frequency::ofMidiNote(pMsg.NoteNumber());
					const float amp = GetPluckAmp((float)pMsg.Velocity() / 127.0f);
					PluckSpectrum(freq.asHz(), amp);

					mNotes.push_back(pMsg);
					break;
				}
				// fallthru to handle note ons that are really note offs

			case IMidiMsg::kNoteOff:
				// remove all notes with the same note number
				for (auto iter = mNotes.begin(); iter != mNotes.end(); ++iter)
				{
					if (iter->NoteNumber() == pMsg.NoteNumber())
					{
						iter = mNotes.erase(iter);
						if (iter == mNotes.end())
						{
							break;
						}
					}
				}
				break;
			}

			mMidiQueue.Remove();
		}


		const float t = (float)GetParam(kCrush)->Value();
		const float crush = expoEaseOut(t, crushBegin, crushChange, kCrushMax - kCrushMin);
		const float rate = (float)GetParam(kPitch)->Value() / 100.0f;
		// convert milliseconds to fractional seconds
		const float decay = (float)GetParam(kDecay)->Int() / 1000.0f;
		//printf("crush with rate %f and depth %f\n", crush, bitCrush.bitRes.getLastValue());
		bitCrush.bitRate.setLastValue(crush);
		tickRate.value.setLastValue(rate);
		// slower tick rate means we have to decrease decay by same ratio so Pitch doesn't change Decay duration
		specGen.decay.setLastValue(decay * rate);
		specGen.brightness.setLastValue(mBrightness);
		specGen.spread.setLastValue(mSpread);

		bitCrush.tick(result, 1);
#ifdef APP_API
		*out1 = result[0] * mGain;
		*out2 = result[0] * mGain;
#else
		*out1 = *in1 + result[0] * mGain;
		*out2 = *in2 + result[0] * mGain;
#endif

    processSpectrum = processSpectrum | specGen.didGenerateOutput();
	}

  if (processSpectrum)
  {
    spectrumCapture.ProcessSpectrum(specGen);
  }

	mMidiQueue.Flush(nFrames);
}

void PLUG_CLASS_NAME::OnIdle()
{
  spectrumCapture.TransmitData(*this);
}

void PLUG_CLASS_NAME::ProcessMidiMsg(const IMidiMsg& msg)
{
	mMidiQueue.Add(msg);
}

void PLUG_CLASS_NAME::OnReset()
{
	TRACE;
	//IMutexLock lock(this);
	specGen.reset();
	bitCrush.setSampleRate((float)GetSampleRate());
	mMidiQueue.Resize(GetBlockSize());
	mNotes.clear();
}

void PLUG_CLASS_NAME::OnParamChange(int paramIdx)
{
	//IMutexLock lock(this);

	switch (paramIdx)
	{
	case kVolume:
		mGain = (float)GetParam(kVolume)->Value() / 100.f;
		break;

	case kPluckX:
	case kPluckY:
	{
		const bool isX = paramIdx == kPluckX;
		const bool isY = paramIdx == kPluckY;
		const float param = (float)GetParam(paramIdx)->GetNormalized();

		if (isX && param != mPluckX)
		{
			Pluck(param, (float)GetParam(kPluckY)->GetNormalized());
			mPluckX = param;
		}
		else if (isY && param != mPluckY)
		{
			Pluck((float)GetParam(kPluckX)->GetNormalized(), param);
			mPluckY = param;
		}
	}
	break;

	case kBandFirst:
	{
		const double bandFirst = GetParam(kBandFirst)->Value();
		const double bandLast = GetParam(kBandLast)->Value();
		if (bandFirst > bandLast - kBandMinDistance)
		{
      const double value = bandLast - kBandMinDistance;
			// this method calls the two methods below and then calls OnParamChanged.
			// we don't need the OnParamChanged call, but it doesn't appear to present a problem.
			// this was changed because InformHostOfParamChanged is a private method in the VST3P build.
			SetParameterValue(kBandFirst, GetParam(kBandFirst)->ToNormalized(value));
			//GetParam(kBandFirst)->Set(value);
			//InformHostOfParamChange(kBandFirst, GetParam(kBandFirst)->GetNormalized());
      SendParameterValueFromAPI(kBandFirst, value, false);
		}		
	}
	break;

	case kBandLast:
	{
		const double bandLast = GetParam(kBandLast)->Value();
		const double bandFirst = GetParam(kBandFirst)->Value();
		if (bandLast < bandFirst + kBandMinDistance)
		{
      const double value = bandFirst + kBandMinDistance;
			// see above for why we call this instead of the two commented out methods
			SetParameterValue(kBandLast, GetParam(kBandLast)->ToNormalized(value));
			//GetParam(kBandLast)->Set(value);
			//InformHostOfParamChange(kBandLast, GetParam(kBandLast)->GetNormalized());
      SendParameterValueFromAPI(kBandLast, value, false);
		}	
	}
	break;

	case kBandSpread:
		mSpread = GetParam(kBandSpread)->Value();
		break;

	case kBrightness:
		mBrightness = (float)GetParam(kBrightness)->Value() / 100.0f;
		break;

	default:
		break;
	}

#if APP_API
	BroadcastParamChange(paramIdx);
#endif
}

void PLUG_CLASS_NAME::Pluck(const float pluckX, const float pluckY)
{
	if (!mIsLoading)
	{
		const float numBands = (float)GetParam(kBandDensity)->Int();		
		if (numBands > 0)
		{
      const float lowFreq = (float)GetParam(kBandFirst)->Value();
      const float hiFreq = (float)GetParam(kBandLast)->Value();
      const float linLogLerp = (float)GetParam(kBandLinLogLerp)->Value();
			for (int b = 0; b <= numBands; ++b)
			{
				const float freq = FrequencyOfString(b, numBands, lowFreq, hiFreq, linLogLerp);
				const float normBand = (float)b / numBands;
				if (fabs(normBand - pluckX) < 0.005f)
				{
					float mag = GetPluckAmp(pluckY);
					PluckSpectrum(freq, mag);
				}
			}
		}
	}
}

void PLUG_CLASS_NAME::PluckSpectrum(const float freq, float mag)
{
	specGen.pluck(freq, mag);
}

float PLUG_CLASS_NAME::GetPluckAmp(const float pluckY) const
{
  return kSpectralAmpMax * pluckY;
}

bool PLUG_CLASS_NAME::OnMessage(int messageTag, int controlTag, int dataSize, const void* pData)
{
  switch (messageTag)
  {
    case kSetMidiMapping:
    {
#if APP_API
      if (dataSize == sizeof(MidiMapping))
      {
        const MidiMapping* map = (MidiMapping*)pData;
        SetControlChangeForParam((IMidiMsg::EControlChangeMsg)map->midiCC, map->param);
        return true;
      }
#endif
    }
    break;

    case kPluckSpectrum:
    {
      if (dataSize == sizeof(PluckCoords))
      {
        const PluckCoords* coords = (PluckCoords*)pData;
        Pluck(coords->x, coords->y);
      }
    }
    break;
  }

  return false;
}
#endif // IPLUG_DSP

#if APP_API
void PLUG_CLASS_NAME::SetControlChangeForParam(const IMidiMsg::EControlChangeMsg cc, const int paramIdx)
{
	controlChangeForParam[paramIdx] = cc;

	char controlName[32];
	sprintf(controlName, "control%u", paramIdx);
	// remove the setting if they unmapped it
	if (cc == kUnmappedParam)
	{
		WritePrivateProfileString(kMidiControlIni, controlName, 0, mIniPath.Get());
	}
	else
	{
		char ccString[100];
		sprintf(ccString, "%u", (unsigned)cc);
		WritePrivateProfileString(kMidiControlIni, controlName, ccString, mIniPath.Get());
	}
}

void PLUG_CLASS_NAME::HandleMidiControlChange(const IMidiMsg& pMsg)
{
	const IMidiMsg::EControlChangeMsg cc = pMsg.ControlChangeIdx();
	for (int i = 0; i < kNumParams; ++i)
	{
		if (controlChangeForParam[i] == cc)
		{
			const double value = pMsg.ControlChange(cc);
      GetParam(i)->SetNormalized(value);
      OnParamChange(i);
      SendParameterValueFromAPI(i, value, true);
		}
	}
}
#endif // APP_API

bool PLUG_CLASS_NAME::OnHostRequestingAboutBox()
{
#if APP_API
#ifdef OS_WIN
	GetUI()->ShowMessageBox(kAboutBoxText, BUNDLE_NAME, kMB_OK);
#else
	// sadly, on osx, ShowMessageBox uses an alert style box that does not show the app icon,
	// which is different from the default About window that is shown.
	// *that* code uses swell's MessageBox, so we use that directly on mac.
	MessageBox(0, kAboutBoxText, BUNDLE_NAME, MB_OK);
#endif
	return true;
#endif
	return false;
}

bool PLUG_CLASS_NAME::OnHostRequestingProductHelp()
{
  WDL_String filename("");
  bool success = false;

#if APP_API
  HostPath(filename);
  if (filename.GetLength())
  {
    filename.Append("/SpectralHarp_manual.pdf");
    success = GetUI()->OpenURL(filename.Get());
  }
#endif

  if (!success)
  {
#if defined(OS_WIN)    
#if UNICODE
    wchar_t wideFilename[1024];
    wchar_t wideIniPath[1024];
    UTF8ToUTF16(wideIniPath, mIniPath.Get(), mIniPath.GetLength());
    GetPrivateProfileString(L"install", L"support path", NULL, wideFilename, 256, wideIniPath);
    UTF16ToUTF8(filename, wideFilename);
#else
    filename.SetLen(256);
    GetPrivateProfileString("install", "support path", NULL, filename.Get(), 256, mIniPath.Get());
#endif
#else
    AppSupportPath(filename, true);
    filename.Append("/Evaluator");
#endif

    filename.Append("/SpectralHarp_manual.pdf");
    success = GetUI()->OpenURL(filename.Get());
  }

  if (!success)
  {
    const char* msg = "Strum the strings by clicking and dragging with the mouse or playing notes on a connected MIDI device.\n\nUse the Spectrum Selection controls to control which part of the spectrum the strings represent.\n\nUse the knobs to adjust the sound.\n\nVisit https://damikyu.itch.io/spectralharp for more info.";
    GetUI()->ShowMessageBox(msg, "Basic Operation", kMB_OK);
  }

  return true;
}

#if APP_API
void PLUG_CLASS_NAME::BroadcastParamChange(const int paramIdx)
{
	// send MIDI CC messages with current param values for any mapped params,
	// which should enable some control surfaces to keep indicators in sync with the UI.
	if (controlChangeForParam[paramIdx] != kUnmappedParam)
	{
		IMidiMsg msg;
		msg.MakeControlChangeMsg(controlChangeForParam[paramIdx], GetParam(paramIdx)->GetNormalized(), 0);
		SendMidiMsg(msg);
	}
}
#endif
