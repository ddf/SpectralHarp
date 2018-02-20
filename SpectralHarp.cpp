#include "SpectralHarp.h"
#include "IPlug_include_in_plug_src.h"
#include "IControl.h"
#include "resource.h"
#include "StringControl.h"
#include "SpectrumSelection.h"
#include "Controls.h"

#if SA_API
#include "app_wrapper/app_main.h"

static const char * kAboutBoxText = "Version " VST3_VER_STR "\nCreated by Damien Quartz\nBuilt on " __DATE__;
#endif

const int kNumPrograms = 1;

enum ELayout
{
	kWidth = GUI_WIDTH,
	kHeight = GUI_HEIGHT,

	kControlPanelHeight = 120,

	kPluckPadHeight = kHeight - kControlPanelHeight,
	kPluckPadMargin = 0,
	kPluckPadSpaceBottom = 5,

	kSpectrumSelect_W = 512,
	kSpectrumSelect_X = kWidth / 2 - kSpectrumSelect_W / 2,
	kSpectrumSelect_Y = kPluckPadHeight + kPluckPadSpaceBottom + 15,
	kSpectrumSelect_H = 15,

	kKnob_X = 0,
	kKnob_Y = kSpectrumSelect_Y + kSpectrumSelect_H + 10,
	kKnob_W = 48,
	kKnob_H = 48,
	
	kKnobCorona = 0,
	kKnobSpacing = 75,

	kVolumeX = 25,
	kBandDensityX = kVolumeX + kKnobSpacing,
	kPitchX = kBandDensityX + kKnobSpacing,
	kDecayX = kPitchX + kKnobSpacing,
	kCrushX = kDecayX + kKnobSpacing,

	kKnobFrames = 60,

	kCaptionT = kKnob_Y + 50,
	kCaptionB = kCaptionT + 15,
	kCaptionW = 50,
};

// background of entire window
static const IColor backColor = IColor(255, 20, 20, 20);
// rectangular panel behind the knobs
static const IColor panelColor = IColor(255, 30, 30, 30);
// text color for labels under the knobs
static const IColor labelColor = IColor(255, 200, 200, 200);
#ifdef OS_WIN
static const int    labelSize  = 12;
#else
static const int    labelSize  = 14;
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
	, mGain(1.)
	, mPluckX(-1)
	, mPluckY(-1)
	, specGen()
	, bitCrush(24, 44100)
	, tickRate(1)
	, highPass(30, 0, Minim::MoogFilter::HP)
	, midiLearnParamIdx(-1)
{
	TRACE;

	for (int i = 0; i < kNumParams; ++i)
	{
		controlChangeForParam[i] = (IMidiMsg::EControlChangeMsg)0;
	}

	//arguments are: name, defaultVal, minVal, maxVal, step, label
	GetParam(kVolume)->InitDouble("Volume", 100., 0., 100.0, 0.1, "%");
	GetParam(kVolume)->SetShape(2.);

	GetParam(kPitch)->InitDouble("Pitch", kPitchDefault, kPitchMin, kPitchMax, 0.1, "%");
	GetParam(kPitch)->SetShape(1.);

	GetParam(kDecay)->InitDouble("Decay", kDecayDefault, kDecayMin, kDecayMax, 0.1, "%");
	GetParam(kDecay)->SetShape(1.);

	GetParam(kCrush)->InitDouble("Crush", kCrushDefault, kCrushMin, kCrushMax, 0.1, "%");
	GetParam(kCrush)->SetShape(1.);

	GetParam(kPluckX)->InitDouble("Pluck X", 0, 0., 100.0, 0.1, "%");
	GetParam(kPluckX)->SetShape(1.);

	GetParam(kPluckY)->InitDouble("Pluck Y", 0, 0., 100.0, 0.1, "%");
	GetParam(kPluckY)->SetShape(1.);

	InitBandParam("First Band", kBandFirst, kBandFirstDefault);
	InitBandParam("Last Band", kBandLast, kBandLastDefault);

	GetParam(kBandDensity)->InitInt("Density", kBandDensityDefault, kBandDensityMin, kBandDensityMax, "strings");

	IGraphics* pGraphics = MakeGraphics(this, kWidth, kHeight);
	pGraphics->AttachPanelBackground(&backColor);

	IText captionText = IText(labelSize, &labelColor);

	pGraphics->AttachControl(new IPanelControl(this, IRECT(0, kPluckPadHeight, kWidth, kHeight), &panelColor));

	// strumming area
	{
		IRECT strumRect = IRECT(kPluckPadMargin, 0, kWidth - kPluckPadMargin, kPluckPadHeight);
		pGraphics->AttachControl(new StringControl(specGen, this, strumRect, 10, kPluckX, kPluckY));

		IText bandLabel = captionText;
		const int capMargin = 14;
		strumRect.B += kPluckPadSpaceBottom;
		bandLabel.mAlign = IText::kAlignNear;
		IRECT lowBandRect = IRECT(strumRect.L + capMargin, strumRect.B, strumRect.L + kCaptionW + capMargin, strumRect.B + 25);
		pGraphics->AttachControl(new ICaptionControl(this, lowBandRect, kBandFirst, &bandLabel, false));

		bandLabel.mAlign = IText::kAlignFar;
		IRECT highBandRect = IRECT(strumRect.R - kCaptionW - capMargin, strumRect.B, strumRect.R - capMargin, strumRect.B + 25);
		pGraphics->AttachControl(new ICaptionControl(this, highBandRect, kBandLast, &bandLabel, false));
	}

	pGraphics->AttachControl(new KnobLineCoronaControl(this, MakeIRectHOffset(kKnob, kVolumeX), kVolume, &knobColor, &knobColor, kKnobCorona));
	pGraphics->AttachControl(new ITextControl(this, IRECT(kVolumeX, kCaptionT, kVolumeX + kCaptionW, kCaptionB), &captionText, "Volume"));

	//pGraphics->AttachControl(new IKnobMultiControl(this, kBandFirstX, kKnobY, kBandFirst, &knob));
	//pGraphics->AttachControl(new ITextControl(this, IRECT(kBandFirstX, kCaptionT, kBandFirstX + kCaptionW, kCaptionB), &captionText, "First"));

	//pGraphics->AttachControl(new IKnobMultiControl(this, kBandLastX, kKnobY, kBandLast, &knob));
	//pGraphics->AttachControl(new ITextControl(this, IRECT(kBandLastX, kCaptionT, kBandLastX + kCaptionW, kCaptionB), &captionText, "Last"));

	// spectrum selection UI
	{
		IRECT rect = MakeIRect(kSpectrumSelect);
		pGraphics->AttachControl(new SpectrumSelection(this, rect, kBandFirst, kBandLast, selectionBackColor, selectionSelectColor, selectionHandleColor));

		rect.T = kPluckPadHeight + kPluckPadSpaceBottom;
		rect.B = rect.T + 10;
		pGraphics->AttachControl(new ITextControl(this, rect, &captionText, "Spectrum Selection"));
	}

	pGraphics->AttachControl(new KnobLineCoronaControl(this, MakeIRectHOffset(kKnob, kBandDensityX), kBandDensity, &knobColor, &knobColor, kKnobCorona));
	pGraphics->AttachControl(new ITextControl(this, IRECT(kBandDensityX, kCaptionT, kBandDensityX + kCaptionW, kCaptionB), &captionText, "Density"));

	//pGraphics->AttachControl(new IKnobMultiControl(this, kSpacingX, kKnobY, kSpacing, &knob));
	//pGraphics->AttachControl(new ITextControl(this, IRECT(kSpacingX, kCaptionT, kSpacingX+kCaptionW, kCaptionB), &captionText, "Spacing"));

	pGraphics->AttachControl(new KnobLineCoronaControl(this, MakeIRectHOffset(kKnob,kPitchX), kPitch, &knobColor, &knobColor, kKnobCorona));
	pGraphics->AttachControl(new ITextControl(this, IRECT(kPitchX, kCaptionT, kPitchX + kCaptionW, kCaptionB), &captionText, "Pitch"));

	pGraphics->AttachControl(new KnobLineCoronaControl(this, MakeIRectHOffset(kKnob,kDecayX),kDecay, &knobColor, &knobColor, kKnobCorona));
	pGraphics->AttachControl(new ITextControl(this, IRECT(kDecayX, kCaptionT, kDecayX + kCaptionW, kCaptionB), &captionText, "Decay"));

	pGraphics->AttachControl(new KnobLineCoronaControl(this, MakeIRectHOffset(kKnob,kCrushX), kCrush, &knobColor, &knobColor, kKnobCorona));
	pGraphics->AttachControl(new ITextControl(this, IRECT(kCrushX, kCaptionT, kCrushX + kCaptionW, kCaptionB), &captionText, "Crush"));

	AttachGraphics(pGraphics);

	//MakePreset("preset 1", ... );
	MakeDefaultPreset((char *) "-", kNumPrograms);

	//-- AUDIO --------------------------------------
	{
		tickRate.value.setLastValue((float)GetParam(kPitch)->Value() / 100.0f);
		tickRate.setInterpolation(true);

		specGen.patch(tickRate).patch(bitCrush);
		bitCrush.setAudioChannelCount(1);
		bitCrush.setSampleRate(44100);
	}
}

SpectralHarp::~SpectralHarp() {}

void SpectralHarp::InitBandParam(const char * name, const int paramIdx, const int defaultValue)
{
	IParam* param = GetParam(paramIdx);
	param->InitInt(name, defaultValue, kBandMin, kBandMax);
	char display[32];
	for (int i = kBandMin; i <= kBandMax; ++i)
	{
		sprintf(display, "%d Hz", (int)specGen.getBandFrequency(i));
		param->SetDisplayText(i, display);
	}
}

void SpectralHarp::ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames)
{
	// Mutex is already locked for us.

	double* in1 = inputs[0];
	double* in2 = inputs[1];
	double* out1 = outputs[0];
	double* out2 = outputs[1];

	float result[1];
	for (int s = 0; s < nFrames; ++s, ++in1, ++in2, ++out1, ++out2)
	{
		while (!mMidiQueue.Empty())
		{
			IMidiMsg* pMsg = mMidiQueue.Peek();
			if (pMsg->mOffset > s) break;

			HandleMidiControlChange(pMsg);

			mMidiQueue.Remove();
		}

		const float t = (float)GetParam(kCrush)->Value();
		const float crush = expoEaseOut(t, 44100, -43100, kCrushMax - kCrushMin);
		//printf("crush with rate %f and depth %f\n", crush, bitCrush.bitRes.getLastValue());
		bitCrush.bitRate.setLastValue(crush);
		tickRate.value.setLastValue((float)GetParam(kPitch)->Value() / 100.0f);
		float decay = Map((float)GetParam(kDecay)->Value(), kDecayMin, kDecayMax, 0.8f, 0.2f);
		specGen.decayRate.setLastValue(decay*decay);

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

void SpectralHarp::BeginMIDILearn(int paramIdx1, int paramIdx2, int x, int y)
{
	if (GetAPI() == kAPIVST3)
	{
		PopupHostContextMenuForParam(paramIdx1, x, y);
	}
	else if ( GetAPI() == kAPISA )
	{
		IPopupMenu menu;
		menu.SetMultiCheck(true);
		WDL_String str;
		if (paramIdx1 != -1)
		{
			str.SetFormatted(64, "MIDI Learn: %s", GetParam(paramIdx1)->GetNameForHost());
			int flags = controlChangeForParam[paramIdx1] ? IPopupMenuItem::kChecked : IPopupMenuItem::kNoFlags;
			menu.AddItem(str.Get(), -1, flags);
		}
		if (paramIdx2 != -1)
		{
			str.SetFormatted(64, "MIDI Learn: %s", GetParam(paramIdx2)->GetNameForHost());
			int flags = controlChangeForParam[paramIdx2] ? IPopupMenuItem::kChecked : IPopupMenuItem::kNoFlags;
			menu.AddItem(str.Get(), -1, flags);
		}
		if (GetGUI()->CreateIPopupMenu(&menu, x, y))
		{
			const int chosen = menu.GetChosenItemIdx();
			if (chosen == 0)
			{			
				if (menu.GetItem(chosen)->GetChecked())
				{
					SetControlChangeForParam((IMidiMsg::EControlChangeMsg)0, paramIdx1);
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
					SetControlChangeForParam((IMidiMsg::EControlChangeMsg)0, paramIdx2);
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

void SpectralHarp::ProcessMidiMsg(IMidiMsg* pMsg)
{
	if (pMsg->StatusMsg() == IMidiMsg::kControlChange)
	{
		const IMidiMsg::EControlChangeMsg cc = pMsg->ControlChangeIdx();
		if (midiLearnParamIdx != -1)
		{
			SetControlChangeForParam(cc, midiLearnParamIdx);
			midiLearnParamIdx = -1;
		}

		mMidiQueue.Add(pMsg);
	}
}

void SpectralHarp::Reset()
{
	TRACE;
	IMutexLock lock(this);
	bitCrush.setSampleRate((float)GetSampleRate());
	mMidiQueue.Resize(GetBlockSize());

#if SA_API
	for (int i = 0; i < kNumParams; ++i)
	{
		controlChangeForParam[i] = (IMidiMsg::EControlChangeMsg)gState->mMidiControlForParam[i];
	}
#endif
}

void SpectralHarp::OnParamChange(int paramIdx)
{
	IMutexLock lock(this);

	switch (paramIdx)
	{
	case kVolume:
		mGain = (float)GetParam(kVolume)->Value() / 100.f;
		break;

	case kPluckX:
		Pluck();
		mPluckX = (float)GetParam(kPluckX)->Value();
		break;

	case kPluckY:
		Pluck();
		mPluckY = (float)GetParam(kPluckY)->Value();
		break;

	case kBandFirst:
	{
		const double bandFirst = GetParam(kBandFirst)->Value();
		const double bandLast = GetParam(kBandLast)->Value();
		if (bandFirst > bandLast - kBandMinDistance)
		{
			GetParam(kBandFirst)->Set(bandLast - kBandMinDistance);
			InformHostOfParamChange(kBandFirst, GetParam(kBandFirst)->GetNormalized());
			GetGUI()->SetParameterFromPlug(kBandFirst, GetParam(kBandFirst)->GetNormalized(), true);
		}
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
			GetGUI()->SetParameterFromPlug(kBandLast, GetParam(kBandLast)->GetNormalized(), true);
		}
	}
	break;

	default:
		break;
	}
}

void SpectralHarp::Pluck()
{
	if (mPluckX != -1 && mPluckY != -1)
	{
		//const int numBands = (Settings::BandLast - Settings::BandFirst) * Settings::BandDensity;
		const float numBands = (float)GetParam(kBandDensity)->Int();
		const float bandFirst = (float)GetParam(kBandFirst)->Int();
		const float bandLast = (float)GetParam(kBandLast)->Int();
		if (numBands > 0)
		{
			const float pluckX = (float)GetParam(kPluckX)->Value();
			const float pluckY = (float)GetParam(kPluckY)->Value();
			for (int b = 0; b <= numBands; ++b)
			{
				const int bindx = (int)roundf(Map((float)b, 0, numBands, bandFirst, bandLast));
				float normBand = Map((float)bindx, bandFirst, bandLast, 0, 100);
				if (fabs(normBand - pluckX) < 0.5f)
				{
					float normY = pluckY / 100.0f;
					float mag = Map(normY, 0, 1, kSpectralAmpMax*0.1f, kSpectralAmpMax);
					specGen.pluck(bindx, mag);
				}
			}
		}
	}
}

void SpectralHarp::SetControlChangeForParam(const IMidiMsg::EControlChangeMsg cc, const int paramIdx)
{
	controlChangeForParam[paramIdx] = cc;
#if SA_API
	gState->mMidiControlForParam[paramIdx] = (UInt16)cc;
	UpdateINI();
#endif
}

void SpectralHarp::HandleMidiControlChange(IMidiMsg* pMsg)
{
	const IMidiMsg::EControlChangeMsg cc = pMsg->ControlChangeIdx();
	for (int i = 0; i < kNumParams; ++i)
	{
		if (controlChangeForParam[i] == cc)
		{
			const double value = pMsg->ControlChange(cc);
			GetParam(i)->SetNormalized(value);
			OnParamChange(i);
			GetGUI()->SetParameterFromPlug(i, GetParam(i)->GetNormalized(), true);
		}
	}
}

bool SpectralHarp::HostRequestingAboutBox()
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

