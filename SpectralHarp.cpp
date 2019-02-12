#include "SpectralHarp.h"
#include "IPlug_include_in_plug_src.h"
#include "IControl.h"
#include "resource.h"
#include "StringControl.h"
#include "SpectrumSelection.h"
#include "KnobLineCoronaControl.h"
#include "TextBox.h"

#include "Frequency.h"

#if SA_API
#include "app_wrapper/app_main.h"

static const char * kAboutBoxText = "Version " VST3_VER_STR "\nCreated by Damien Quartz\nBuilt on " __DATE__;
#endif

const int kNumPrograms = 1;

// name of the section of the INI file we save midi cc mappings to
const char * kMidiControlIni = "midi";
const IMidiMsg::EControlChangeMsg kUnmappedParam = (IMidiMsg::EControlChangeMsg)128;

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

	kCaptionT = kKnob_Y + 50,
	kCaptionB = kCaptionT + 15,
	kCaptionW = 50,

	kTextBoxW = 64,
	kTextBoxH = kSpectrumSelect_H + 2,
	
	kTitleX = 10,
	kTitleBottomMargin = 15
};

// background of entire window
static const IColor backColor = IColor(255, 15, 15, 15);
// rectangular panel behind the knobs
static const IColor panelColor = IColor(255, 30, 30, 30);
// color of shadow cast on strings by the knob panel
static const IColor shadowColor = IColor(200, 10, 10, 10);
// text color for labels under the knobs
static const IColor labelColor = IColor(255, 200, 200, 200);
#ifdef OS_WIN
static const int    labelSize  = 12;
#else
static const int    labelSize  = 14;
#endif
// text style
static const IColor titleColor = IColor(255, 60, 60, 60);
static const int titleSize = 16;
static const char * titleString = PLUG_NAME " " PLUG_VERSION_STR;
// #TODO how to we access these system fonts now? or do we just roll with Roboto?
#if defined(OS_WIN)
static const char* titleFontName = nullptr; // "Segoe UI";
#else
static const char* titleFontName = nullptr; // "Helvetica Neue";
#endif

// spectrum selection colors
static const IColor selectionBackColor = backColor;
static const IColor selectionSelectColor = IColor(255, 80, 80, 80);
static const IColor selectionHandleColor = IColor(255, 200, 200, 200);

// knob colors
static const IColor knobColor = IColor(255, 200, 200, 200);

float expoEaseOut(float t, float b, float c, float d)
{
	return c * (-powf(2, -10 * t / d) + 1) + b;
}

SpectralHarp::SpectralHarp(IPlugInstanceInfo instanceInfo)
	: IPLUG_CTOR(kNumParams, kNumPrograms, instanceInfo)
	, mIsLoading(false)
	, mGain(1.)
	, mPluckX(0)
	, mPluckY(0)
	, mSpread(0)
	, mBrightness(0)
	, specGen()
	, bitCrush(24, 44100)
	, tickRate(1)
	, midiLearnParamIdx(-1)
{
	TRACE;

	for (int i = 0; i < kNumParams; ++i)
	{
		controlChangeForParam[i] = kUnmappedParam;
	}

	mNotes.reserve(32);

	//arguments are: name, defaultVal, minVal, maxVal, step, label
	GetParam(kVolume)->InitDouble("Volume", 25.0, 0.0, 100.0, 0.1, "%");
	//GetParam(kVolume)->SetShape(2.);

	GetParam(kPitch)->InitDouble("Pitch", kPitchDefault, kPitchMin, kPitchMax, 0.1, "%");
	//GetParam(kPitch)->SetShape(1.);

	GetParam(kDecay)->InitInt("Decay", kDecayDefault, kDecayMin, kDecayMax, "ms");
	//GetParam(kDecay)->SetShape(1.);

	GetParam(kCrush)->InitDouble("Crush", kCrushDefault, kCrushMin, kCrushMax, 0.1, "%");
	//GetParam(kCrush)->SetShape(1.);

	GetParam(kPluckX)->InitDouble("Pluck X", 0, 0., 100.0, 0.1, "%");
	//GetParam(kPluckX)->SetShape(1.);

	GetParam(kPluckY)->InitDouble("Pluck Y", 100.0, 0., 100.0, 0.1, "%");
	//GetParam(kPluckY)->SetShape(1.);

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
    pGraphics->AttachPanelBackground(backColor);
    pGraphics->LoadFont(ROBOTTO_FN);

    IText captionText = IText(labelSize, labelColor);
    captionText.mTextEntryBGColor = backColor;
    captionText.mTextEntryFGColor = labelColor;

    IRECT strumRect = IRECT(kPluckPadMargin, 0, kWidth - kPluckPadMargin, kPluckPadHeight);
    pGraphics->AttachControl(new StringControl(specGen, strumRect, 10));

    IRECT stringShadowRect = IRECT(0, kPluckPadHeight - 5, kWidth, kPluckPadHeight);
    pGraphics->AttachControl(new IPanelControl(stringShadowRect, shadowColor));

    pGraphics->AttachControl(new IPanelControl(IRECT(0, kPluckPadHeight, kWidth, kHeight), panelColor));

    const int arrowMargin = 14;
    IRECT arrowRect = IRECT(strumRect.L + arrowMargin, strumRect.B, strumRect.R - arrowMargin, strumRect.B + kSpectrumSelect_H + kPluckPadSpaceBottom + 5);
    pGraphics->AttachControl(new SpectrumArrows(arrowRect, labelColor));

    // string Hz labels
    {
      IText bandLabel = captionText;
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
    knob->SetLabelControl(text);
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
    knob->SetLabelControl(text);
    pGraphics->AttachControl(knob);
    pGraphics->AttachControl(text);

    knob = new KnobLineCoronaControl(MakeIRectHOffset(kKnob, kTuningX), kBandLinLogLerp, knobColor, knobColor, kKnobLineThickness);
    text = new ITextControl(IRECT(kTuningX, kCaptionT, kTuningX + kCaptionW, kCaptionB), "Tuning", captionText);
    knob->SetLabelControl(text);
    pGraphics->AttachControl(knob);
    pGraphics->AttachControl(text);

    knob = new KnobLineCoronaControl(MakeIRectHOffset(kKnob, kSpreadX), kBandSpread, knobColor, knobColor, kKnobLineThickness);
    text = new ITextControl(IRECT(kSpreadX, kCaptionT, kSpreadX + kCaptionW, kCaptionB), "Spread", captionText);
    knob->SetLabelControl(text);
    pGraphics->AttachControl(knob);
    pGraphics->AttachControl(text);

    knob = new KnobLineCoronaControl(MakeIRectHOffset(kKnob, kBrightnessX), kBrightness, knobColor, knobColor, kKnobLineThickness);
    text = new ITextControl(IRECT(kBrightnessX, kCaptionT, kBrightnessX + kCaptionW, kCaptionB), "Brightness", captionText);
    knob->SetLabelControl(text);
    pGraphics->AttachControl(knob);
    pGraphics->AttachControl(text);

    knob = new KnobLineCoronaControl(MakeIRectHOffset(kKnob, kPitchX), kPitch, knobColor, knobColor, kKnobLineThickness);
    text = new ITextControl(IRECT(kPitchX, kCaptionT, kPitchX + kCaptionW, kCaptionB), "Pitch", captionText);
    knob->SetLabelControl(text);
    pGraphics->AttachControl(knob);
    pGraphics->AttachControl(text);

    knob = new KnobLineCoronaControl(MakeIRectHOffset(kKnob, kDecayX), kDecay, knobColor, knobColor, kKnobLineThickness);
    text = new ITextControl(IRECT(kDecayX, kCaptionT, kDecayX + kCaptionW, kCaptionB), "Decay", captionText);
    knob->SetLabelControl(text);
    pGraphics->AttachControl(knob);
    pGraphics->AttachControl(text);

    knob = new KnobLineCoronaControl(MakeIRectHOffset(kKnob, kCrushX), kCrush, knobColor, knobColor, kKnobLineThickness);
    text = new ITextControl(IRECT(kCrushX, kCaptionT, kCrushX + kCaptionW, kCaptionB), "Crush", captionText);
    knob->SetLabelControl(text);
    pGraphics->AttachControl(knob);
    pGraphics->AttachControl(text);

    // logooo

    IText titleText(titleSize, titleColor, titleFontName);
    titleText.mAlign = IText::kAlignNear;
    IRECT titleRect(kTitleX, PLUG_HEIGHT - kTitleBottomMargin - 10, 150, PLUG_HEIGHT - kTitleBottomMargin);
    pGraphics->MeasureText(titleText, titleString, titleRect);
    pGraphics->AttachControl(new ITextControl(titleRect, titleString, titleText));
  };

#endif // IPLUG_EDITOR

  // do we stil need this?
	//MakeDefaultPreset((char *) "-", kNumPrograms);

	//-- AUDIO --------------------------------------
#if IPLUG_DSP
  tickRate.value.setLastValue((float)GetParam(kPitch)->Value() / 100.0f);
  tickRate.setInterpolation(true);

  specGen.patch(tickRate).patch(bitCrush);
  bitCrush.setAudioChannelCount(1);
  bitCrush.setSampleRate(44100);
#endif
}

SpectralHarp::~SpectralHarp() {}

void SpectralHarp::InitBandParam(const char * name, const int paramIdx, const int defaultValue)
{
	IParam* param = GetParam(paramIdx);
	param->InitInt(name, defaultValue, kBandMin, kBandMax, "Hz");
}

void SpectralHarp::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
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

	float result[1];
	for (int s = 0; s < nFrames; ++s, ++in1, ++in2, ++out1, ++out2)
	{
		while (!mMidiQueue.Empty())
		{
			IMidiMsg& pMsg = mMidiQueue.Peek();
			if (pMsg.mOffset > s) break;

			switch (pMsg.StatusMsg())
			{
			case IMidiMsg::kControlChange:
				HandleMidiControlChange(pMsg);
				break;

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
#ifdef SA_API
		*out1 = result[0] * mGain;
		*out2 = result[0] * mGain;
#else
		*out1 = *in1 + result[0] * mGain;
		*out2 = *in2 + result[0] * mGain;
#endif
	}

	mMidiQueue.Flush(nFrames);
}

void SpectralHarp::BeginMIDILearn(int controlID, int paramIdx1, int paramIdx2, float x, float y)
{
	if (GetAPI() == kAPIVST3)
	{
// in Reaper on OSX the popup's Y position is inverted.
// not sure if this is true in other hosts on OSX, so we only modify for Reaper.
#ifdef OS_OSX
		if ( GetHost() == kHostReaper )
		{
			y = GUI_HEIGHT - y;
		}
#endif

    if (GetUI() != nullptr)
    {
      GetUI()->PopupHostContextMenuForParam(controlID, paramIdx1, x, y);
    }    
	}
	else if ( GetAPI() == kAPIAPP )
	{
		IPopupMenu menu;
		menu.SetMultiCheck(true);
		WDL_String str;
		if (paramIdx1 != -1)
		{
			bool isMapped = controlChangeForParam[paramIdx1] != kUnmappedParam;
			int flags = isMapped ? IPopupMenu::Item::kChecked : IPopupMenu::Item::kNoFlags;
			if (isMapped)
			{
				str.SetFormatted(64, "MIDI Learn: %s (CC %d)", GetParam(paramIdx1)->GetNameForHost(), (int)controlChangeForParam[paramIdx1]);
			}
			else
			{
				str.SetFormatted(64, "MIDI Learn: %s", GetParam(paramIdx1)->GetNameForHost());
			}
			menu.AddItem(str.Get(), -1, flags);
		}
		if (paramIdx2 != -1)
		{
			bool isMapped = controlChangeForParam[paramIdx2] != kUnmappedParam;
			int flags = isMapped ? IPopupMenu::Item::kChecked : IPopupMenu::Item::kNoFlags;
			if (isMapped)
			{
				str.SetFormatted(64, "MIDI Learn: %s (CC %d)", GetParam(paramIdx2)->GetNameForHost(), (int)controlChangeForParam[paramIdx2]);
			}
			else
			{
				str.SetFormatted(64, "MIDI Learn: %s", GetParam(paramIdx2)->GetNameForHost());
			}
			menu.AddItem(str.Get(), -1, flags);
		}
		if (GetUI()->CreatePopupMenu(menu, x, y))
		{
			const int chosen = menu.GetChosenItemIdx();
			if (chosen == 0)
			{			
				if (menu.GetItem(chosen)->GetChecked())
				{
					SetControlChangeForParam(kUnmappedParam, paramIdx1);
				}
				else
				{
					midiLearnParamIdx = paramIdx1;
				}
			}
			else if (chosen == 1)
			{
				if (menu.GetItem(chosen)->GetChecked())
				{
					SetControlChangeForParam(kUnmappedParam, paramIdx2);
				}
				else
				{
					midiLearnParamIdx = paramIdx2;
				}
			}
			else
			{
 				midiLearnParamIdx = -1;
			}
		}
	}
}

void SpectralHarp::ProcessMidiMsg(const IMidiMsg& msg)
{
	if (msg.StatusMsg() == IMidiMsg::kControlChange)
	{
		const IMidiMsg::EControlChangeMsg cc = msg.ControlChangeIdx();
		if (midiLearnParamIdx != -1)
		{
			SetControlChangeForParam(cc, midiLearnParamIdx);
			midiLearnParamIdx = -1;
		}
	}
	mMidiQueue.Add(msg);
}

void SpectralHarp::OnReset()
{
	TRACE;
	//IMutexLock lock(this);
	specGen.reset();
	bitCrush.setSampleRate((float)GetSampleRate());
	mMidiQueue.Resize(GetBlockSize());
	mNotes.clear();

	// read control mappings from the INI if we are running standalone
#if SA_API
	for (int i = 0; i < kNumParams; ++i)
	{
		int defaultMapping = (int)controlChangeForParam[i];
		char controlName[32];
		sprintf(controlName, "control%u", i);
		controlChangeForParam[i] = (IMidiMsg::EControlChangeMsg)GetPrivateProfileInt(kMidiControlIni, controlName, defaultMapping, gINIPath);

		BroadcastParamChange(i);
	}
#endif
}

int SpectralHarp::UnserializeState(const IByteChunk& chunk, int startPos)
{
	TRACE;
	//IMutexLock lock(this);
	
	mIsLoading = true;
	int endPos = UnserializeParams(chunk, startPos);
	mIsLoading = false;
	return endPos;
}

void SpectralHarp::OnParamChange(int paramIdx)
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
			GetParam(kBandFirst)->Set(bandLast - kBandMinDistance);
			InformHostOfParamChange(kBandFirst, GetParam(kBandFirst)->GetNormalized());
		}		
		// we always push this back to the UI because we have multiple UI elements for this param
    // #TODO how to push this back now?
		//GetGUI()->SetParameterFromPlug(kBandFirst, GetParam(kBandFirst)->GetNormalized(), true);
	}
	break;

	case kBandLast:
	{
		const double bandLast = GetParam(kBandLast)->Value();
		const double bandFirst = GetParam(kBandFirst)->Value();
		if (bandLast < bandFirst + kBandMinDistance)
		{
			GetParam(kBandLast)->Set(bandFirst + kBandMinDistance);
			InformHostOfParamChange(kBandLast, GetParam(kBandLast)->GetNormalized());			
		}	
		// we always push this back to the UI because we have multiple UI elements for this param
    // #TODO how to push this back now?
		//GetGUI()->SetParameterFromPlug(kBandLast, GetParam(kBandLast)->GetNormalized(), true);
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

	BroadcastParamChange(paramIdx);
}


float SpectralHarp::FrequencyOfString(int stringNum)
{
	const double t = (double)stringNum / GetParam(kBandDensity)->Value();
	// convert first and last bands to midi notes and then do a linear interp, converting back to Hz at the end.
	Minim::Frequency lowFreq = Minim::Frequency::ofHertz(GetParam(kBandFirst)->Value());
	Minim::Frequency hiFreq = Minim::Frequency::ofHertz(GetParam(kBandLast)->Value());
	const float linFreq = Map(t, 0, 1, lowFreq.asHz(), hiFreq.asHz());
	const float midiNote = Map(t, 0, 1, lowFreq.asMidiNote(), hiFreq.asMidiNote());
	const float logFreq = Minim::Frequency::ofMidiNote(midiNote).asHz();
	// we lerp from logFreq up to linFreq because log spacing clusters frequencies
	// towards the bottom of the range, which means that when holding down the mouse on a string
	// and lowering this param, you'll hear the pitch drop, which makes more sense than vice-versa.
	return Map(GetParam(kBandLinLogLerp)->Value(), 0, 1, logFreq, linFreq);
}

void SpectralHarp::Pluck(const float pluckX, const float pluckY)
{
	if (!mIsLoading)
	{
		const float numBands = (float)GetParam(kBandDensity)->Int();		
		if (numBands > 0)
		{
			for (int b = 0; b <= numBands; ++b)
			{
				const float freq = FrequencyOfString(b);
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

void SpectralHarp::PluckSpectrum(const float freq, float mag)
{
	specGen.pluck(freq, mag);
}

void SpectralHarp::SetControlChangeForParam(const IMidiMsg::EControlChangeMsg cc, const int paramIdx)
{
	controlChangeForParam[paramIdx] = cc;

#if SA_API
	char controlName[32];
	sprintf(controlName, "control%u", paramIdx);
	// remove the setting if they unmapped it
	if (cc == kUnmappedParam)
	{
		WritePrivateProfileString(kMidiControlIni, controlName, 0, gINIPath);
	}
	else
	{
		char ccString[100];
		sprintf(ccString, "%u", (unsigned)cc);
		WritePrivateProfileString(kMidiControlIni, controlName, ccString, gINIPath);
	}
#endif
}

float SpectralHarp::GetPluckAmp(const float pluckY) const
{
	return kSpectralAmpMax*pluckY;
}

void SpectralHarp::HandleMidiControlChange(const IMidiMsg& pMsg)
{
	const IMidiMsg::EControlChangeMsg cc = pMsg.ControlChangeIdx();
	for (int i = 0; i < kNumParams; ++i)
	{
		if (controlChangeForParam[i] == cc)
		{
			const double value = pMsg.ControlChange(cc);
			GetParam(i)->SetNormalized(value);
			OnParamChange(i);
			//GetGUI()->SetParameterFromPlug(i, GetParam(i)->GetNormalized(), true);
		}
	}
}

bool SpectralHarp::OnHostRequestingAboutBox()
{
#if SA_API
#ifdef OS_WIN
	GetGUI()->ShowMessageBox(kAboutBoxText, BUNDLE_NAME, MB_OK);
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

void SpectralHarp::BroadcastParamChange(const int paramIdx)
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
