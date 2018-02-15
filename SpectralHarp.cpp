#include "SpectralHarp.h"
#include "IPlug_include_in_plug_src.h"
#include "IControl.h"
#include "resource.h"
#include "src/Settings.h"
#include "StringControl.h"
#include "SpectrumSelection.h"

const int kNumPrograms = 1;

enum EParams
{
	kGain = 0,
	kSpacing, // unused now
	kPitch,
	kDecay,
	kCrush,
	// params for the xy pad that can be used to "strum"
	kPluckX,
	kPluckY,
	kBandFirst,
	kBandLast,
	kBandDensity,
	kNumParams
};

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

	kKnobSpacing = 75,
	kKnobY = kSpectrumSelect_Y + kSpectrumSelect_H + 10,

	kVolumeX = 25,
	kBandDensityX = kVolumeX + kKnobSpacing,
	kPitchX = kBandDensityX + kKnobSpacing,
	kDecayX = kPitchX + kKnobSpacing,
	kCrushX = kDecayX + kKnobSpacing,

	kKnobFrames = 60,

	kCaptionT = kKnobY + 50,
	kCaptionB = kCaptionT + 15,
	kCaptionW = 50,
};

// background of entire window
static const IColor backColor = IColor(255, 20, 20, 20);
// rectangluar panel behind the knobs
static const IColor panelColor = IColor(255, 30, 30, 30);
// text color for labels under the knobs
static const IColor labelColor = COLOR_GRAY;

// spectrum selection colors
static const IColor selectionBackColor = backColor;
static const IColor selectionSelectColor = IColor(255, 80, 80, 80);
static const IColor selectionHandleColor = IColor(255, 200, 200, 200);


float expoEaseOut(float t, float b, float c, float d)
{
	return c * (-powf(2, -10 * t / d) + 1) + b;
}

void computeLastBand()
{
	//int last = Settings::BandOffset + 128 * Settings::BandSpacing;
	//Settings::BandLast = last < 3 ? 3 : (int)fmin(last, kSpectralGenSize / 4);
}

void bandSpacingChanged(float value)
{
	//Settings::BandSpacing = (int)value;
	//computeLastBand();
}

void bandOffsetChanged(float value)
{
	Settings::BandOffset = (int)value;
	computeLastBand();
}

void decayChanged(float value)
{
	Settings::Decay = value;
	SpectralGen::decay = value*value;
}

void bitCrushChanged(float value)
{
	Settings::BitCrush = value;
}

void pitchChanged(float value)
{
	Settings::Pitch = value;
}

SpectralHarp::SpectralHarp(IPlugInstanceInfo instanceInfo)
	: IPLUG_CTOR(kNumParams, kNumPrograms, instanceInfo)
	, mGain(1.)
	, mPluckX(-1)
	, mPluckY(-1)
	, specGen()
	, bitCrush(24, Settings::BitCrush)
	, tickRate(Settings::Pitch)
	, highPass(30, 0, Minim::MoogFilter::HP)
{
	TRACE;

	//arguments are: name, defaultVal, minVal, maxVal, step, label
	GetParam(kGain)->InitDouble("Volume", 100., 0., 100.0, 0.01, "%");
	GetParam(kGain)->SetShape(2.);

	GetParam(kPitch)->InitDouble("Pitch", Settings::Pitch, Settings::PitchMin, Settings::PitchMax, 0.01, "%");
	GetParam(kPitch)->SetShape(1.);

	GetParam(kDecay)->InitDouble("Decay", Settings::Map(Settings::Decay, Settings::DecayMax, Settings::DecayMin, 0, 1), 0, 1, 0.01, "%");
	GetParam(kDecay)->SetShape(1.);

	GetParam(kCrush)->InitDouble("Crush", 0, 0, 1, 0.01, "%");
	GetParam(kCrush)->SetShape(1.);

	GetParam(kPluckX)->InitDouble("PluckX", 0, 0., 100.0, 0.01, "%");
	GetParam(kPluckX)->SetShape(1.);

	GetParam(kPluckY)->InitDouble("PluckY", 0, 0., 100.0, 0.01, "%");
	GetParam(kPluckY)->SetShape(1.);

	InitBandParam("BandFirst", kBandFirst, Settings::BandFirst);
	InitBandParam("BandLast", kBandLast, Settings::BandLast);

	GetParam(kBandDensity)->InitInt("BandDensity", Settings::BandDensity, Settings::BandDensityMin, Settings::BandDesityMax, "strings");

	IGraphics* pGraphics = MakeGraphics(this, kWidth, kHeight);
	pGraphics->AttachPanelBackground(&backColor);

	IBitmap knob = pGraphics->LoadIBitmap(KNOB_ID, KNOB_FN, kKnobFrames);
	IText captionText = IText(&labelColor);

	pGraphics->AttachControl(new IPanelControl(this, IRECT(0, kPluckPadHeight, kWidth, kHeight), &panelColor));

	// strumming area
	{
		IRECT strumRect = IRECT(kPluckPadMargin, 0, kWidth - kPluckPadMargin, kPluckPadHeight);
		pGraphics->AttachControl(new StringControl(specGen, this, strumRect, 10, kPluckX, kPluckY));

		IText bandLabel = captionText;
		const int capMargin = 2;
		strumRect.B += kPluckPadSpaceBottom;
		bandLabel.mAlign = IText::kAlignNear;
		IRECT lowBandRect = IRECT(strumRect.L + capMargin, strumRect.B, strumRect.L + kCaptionW + capMargin, strumRect.B + 25);
		pGraphics->AttachControl(new ICaptionControl(this, lowBandRect, kBandFirst, &bandLabel));

		bandLabel.mAlign = IText::kAlignFar;
		IRECT highBandRect = IRECT(strumRect.R - kCaptionW - capMargin, strumRect.B, strumRect.R - capMargin, strumRect.B + 25);
		pGraphics->AttachControl(new ICaptionControl(this, highBandRect, kBandLast, &bandLabel));
	}

	pGraphics->AttachControl(new IKnobMultiControl(this, kVolumeX, kKnobY, kGain, &knob));
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

	pGraphics->AttachControl(new IKnobMultiControl(this, kBandDensityX, kKnobY, kBandDensity, &knob));
	pGraphics->AttachControl(new ITextControl(this, IRECT(kBandDensityX, kCaptionT, kBandDensityX + kCaptionW, kCaptionB), &captionText, "Density"));

	//pGraphics->AttachControl(new IKnobMultiControl(this, kSpacingX, kKnobY, kSpacing, &knob));
	//pGraphics->AttachControl(new ITextControl(this, IRECT(kSpacingX, kCaptionT, kSpacingX+kCaptionW, kCaptionB), &captionText, "Spacing"));

	pGraphics->AttachControl(new IKnobMultiControl(this, kPitchX, kKnobY, kPitch, &knob));
	pGraphics->AttachControl(new ITextControl(this, IRECT(kPitchX, kCaptionT, kPitchX + kCaptionW, kCaptionB), &captionText, "Pitch"));

	pGraphics->AttachControl(new IKnobMultiControl(this, kDecayX, kKnobY, kDecay, &knob));
	pGraphics->AttachControl(new ITextControl(this, IRECT(kDecayX, kCaptionT, kDecayX + kCaptionW, kCaptionB), &captionText, "Decay"));

	pGraphics->AttachControl(new IKnobMultiControl(this, kCrushX, kKnobY, kCrush, &knob));
	pGraphics->AttachControl(new ITextControl(this, IRECT(kCrushX, kCaptionT, kCrushX + kCaptionW, kCaptionB), &captionText, "Crush"));

	AttachGraphics(pGraphics);

	//MakePreset("preset 1", ... );
	MakeDefaultPreset((char *) "-", kNumPrograms);

	//-- AUDIO --------------------------------------
	{
		computeLastBand();
		decayChanged(Settings::Decay);

		tickRate.value.setLastValue(Settings::Pitch);
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
	param->InitInt(name, defaultValue, Settings::BandMin, Settings::BandMax);
	char display[32];
	for (int i = Settings::BandMin; i <= Settings::BandMax; ++i)
	{
		sprintf(display, "%d Hz", (int)specGen.getBandFrequency(i));
		param->SetDisplayText(i, display);
	}
}

void SpectralHarp::ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames)
{
	// Mutex is already locked for us.

	float t = Settings::Map(Settings::BitCrush, Settings::BitCrushMin, Settings::BitCrushMax, 0, 1);
	float crush = expoEaseOut(t, Settings::BitCrushMin, Settings::BitCrushMax - Settings::BitCrushMin, 1);
	//printf( "crush with rate %f and depth %f\n", crush, bitCrush.bitRes.getLastValue() );
	bitCrush.bitRate.setLastValue(crush);
	tickRate.value.setLastValue(Settings::Pitch);

	double* in1 = inputs[0];
	double* in2 = inputs[1];
	double* out1 = outputs[0];
	double* out2 = outputs[1];

	for (int s = 0; s < nFrames; ++s, ++in1, ++in2, ++out1, ++out2)
	{
		float result[1];
		bitCrush.tick(result, 1);
		*out1 = *in1 + result[0] * mGain;
		*out2 = *in2 + result[0] * mGain;
	}
}

void SpectralHarp::Reset()
{
	TRACE;
	IMutexLock lock(this);
	bitCrush.setSampleRate((float)GetSampleRate());
}

void SpectralHarp::OnParamChange(int paramIdx)
{
	IMutexLock lock(this);

	switch (paramIdx)
	{
	case kGain:
		mGain = (float)GetParam(kGain)->Value() / 100.f;
		break;

	case kSpacing:
		bandSpacingChanged((float)GetParam(kSpacing)->Value());
		break;

	case kPitch:
		pitchChanged((float)GetParam(kPitch)->Value());
		break;

	case kCrush:
		bitCrushChanged(Settings::Map((float)GetParam(kCrush)->Value(), 0, 1, Settings::BitCrushMin, Settings::BitCrushMax));
		break;

	case kDecay:
		decayChanged(Settings::Map((float)GetParam(kDecay)->Value(), 0, 1, Settings::DecayMin, Settings::DecayMax));
		break;

	case kPluckX:
		pluck();
		mPluckX = (float)GetParam(kPluckX)->Value();
		break;

	case kPluckY:
		pluck();
		mPluckY = (float)GetParam(kPluckY)->Value();
		break;

	case kBandFirst:
		Settings::BandFirst = (int)GetParam(kBandFirst)->Value();
		break;

	case kBandLast:
		Settings::BandLast = (int)GetParam(kBandLast)->Value();
		break;

	case kBandDensity:
		Settings::BandDensity = (int)GetParam(kBandDensity)->Value();
		break;

	default:
		break;
	}
}

void SpectralHarp::pluck()
{
	if (mPluckX != -1 && mPluckY != -1)
	{
		//const int numBands = (Settings::BandLast - Settings::BandFirst) * Settings::BandDensity;
		const int numBands = Settings::BandDensity;
		if (numBands > 0)
		{
			const float pluckX = (float)GetParam(kPluckX)->Value();
			const float pluckY = (float)GetParam(kPluckY)->Value();
			for (int b = 0; b <= numBands; ++b)
			{
				const int bindx = (int)roundf(Settings::Map((float)b, 0, (float)numBands, Settings::BandFirst, Settings::BandLast));
				float normBand = Settings::Map((float)bindx, (float)Settings::BandFirst, (float)Settings::BandLast, 0, 100);
				if (fabs(normBand - pluckX) < 0.5f)
				{
					float normY = pluckY / 100.0f;
					float mag = Settings::Map(normY, 0, 1, Settings::SpectralAmpMax*0.1f, Settings::SpectralAmpMax);
					specGen.pluck(bindx, mag);
				}
			}
		}
	}
}
