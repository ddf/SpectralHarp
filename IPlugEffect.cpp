#include "IPlugEffect.h"
#include "IPlug_include_in_plug_src.h"
#include "IControl.h"
#include "resource.h"
#include "src/Settings.h"
#include "StringControl.h"

const int kNumPrograms = 1;

enum EParams
{
	kGain = 0,
	kSpacing,
	kPitch,
	kDecay,
	kCrush,
	// params for the xy pad that can be used to "strum"
	kPluckX,
	kPluckY,
	kNumParams
};

enum ELayout
{
	kWidth = GUI_WIDTH,
	kHeight = GUI_HEIGHT,

	kKnobSpacing = 75,

	kGainX = 25,
	kSpacingX = kGainX + kKnobSpacing,
	kPitchX = kSpacingX + kKnobSpacing,
	kDecayX = kPitchX + kKnobSpacing,
	kCrushX = kDecayX + kKnobSpacing,
	kKnobY = GUI_HEIGHT - 70,
	kKnobFrames = 60,
  
  kCaptionT = GUI_HEIGHT - 20,
  kCaptionB = GUI_HEIGHT,
  kCaptionW = 50,

	kPluckPadHeight = kKnobY,
	kPluckPadMargin = 0
};

float expoEaseOut(float t, float b, float c, float d)
{
	return c * (-powf(2, -10 * t / d) + 1) + b;
}

void computeLastBand()
{
	int last = Settings::BandOffset + 128 * Settings::BandSpacing;
	Settings::BandLast = last < 3 ? 3 : fmin(last, kSpectralGenSize / 4);
}

void bandSpacingChanged(float value)
{
	Settings::BandSpacing = value;
	computeLastBand();
}

void bandOffsetChanged(float value)
{
	Settings::BandOffset = value;
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

float map(float value, float istart, float istop, float ostart, float ostop) 
{
	return ostart + (ostop - ostart) * ((value - istart) / (istop - istart));
}

IPlugEffect::IPlugEffect(IPlugInstanceInfo instanceInfo)
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

  GetParam(kSpacing)->InitDouble("Spacing", Settings::BandSpacing, Settings::BandSpacingMin, Settings::BandSpacingMax, 0.01, "%");
  GetParam(kSpacing)->SetShape(1.);

  GetParam(kPitch)->InitDouble("Pitch", Settings::Pitch, Settings::PitchMin, Settings::PitchMax, 0.01, "%");
  GetParam(kPitch)->SetShape(1.);

  GetParam(kDecay)->InitDouble("Decay", map(Settings::Decay, Settings::DecayMax, Settings::DecayMin, 0, 1), 0, 1, 0.01, "%");
  GetParam(kDecay)->SetShape(1.);

  GetParam(kCrush)->InitDouble("Crush", 0, 0, 1, 0.01, "%");
  GetParam(kCrush)->SetShape(1.);

  GetParam(kPluckX)->InitDouble("PluckX", 0, 0., 100.0, 0.01, "%");
  GetParam(kPluckX)->SetShape(1.);

  GetParam(kPluckY)->InitDouble("PluckY", 0, 0., 100.0, 0.01, "%");
  GetParam(kPluckY)->SetShape(1.);

  IGraphics* pGraphics = MakeGraphics(this, kWidth, kHeight);
  pGraphics->AttachPanelBackground(&COLOR_BLACK);

  IBitmap knob = pGraphics->LoadIBitmap(KNOB_ID, KNOB_FN, kKnobFrames);
  IText captionText = IText(&COLOR_WHITE);

  pGraphics->AttachControl(new IKnobMultiControl(this, kGainX, kKnobY, kGain, &knob));
  pGraphics->AttachControl(new ITextControl(this, IRECT(kGainX, kCaptionT, kGainX+kCaptionW, kCaptionB), &captionText, "Volume"));
  pGraphics->AttachControl(new IKnobMultiControl(this, kSpacingX, kKnobY, kSpacing, &knob));
  pGraphics->AttachControl(new ITextControl(this, IRECT(kSpacingX, kCaptionT, kSpacingX+kCaptionW, kCaptionB), &captionText, "Spacing"));
  pGraphics->AttachControl(new IKnobMultiControl(this, kPitchX, kKnobY, kPitch, &knob));
  pGraphics->AttachControl(new ITextControl(this, IRECT(kPitchX, kCaptionT, kPitchX+kCaptionW, kCaptionB), &captionText, "Pitch"));
  pGraphics->AttachControl(new IKnobMultiControl(this, kDecayX, kKnobY, kDecay, &knob));
  pGraphics->AttachControl(new ITextControl(this, IRECT(kDecayX, kCaptionT, kDecayX+kCaptionW, kCaptionB), &captionText, "Decay"));
  pGraphics->AttachControl(new IKnobMultiControl(this, kCrushX, kKnobY, kCrush, &knob));
  pGraphics->AttachControl(new ITextControl(this, IRECT(kCrushX, kCaptionT, kCrushX+kCaptionW, kCaptionB), &captionText, "Crush"));
  
  pGraphics->AttachControl(new StringControl(specGen, this, IRECT(kPluckPadMargin, 0, kWidth - kPluckPadMargin, kPluckPadHeight), 10, kPluckX, kPluckY));

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

IPlugEffect::~IPlugEffect() {}

void IPlugEffect::ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames)
{
  // Mutex is already locked for us.

	float t = map(Settings::BitCrush, Settings::BitCrushMin, Settings::BitCrushMax, 0, 1);
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

void IPlugEffect::Reset()
{
  TRACE;
  IMutexLock lock(this);
}

void IPlugEffect::OnParamChange(int paramIdx)
{
  IMutexLock lock(this);

  switch (paramIdx)
  {
    case kGain:
      mGain = GetParam(kGain)->Value() / 100.;
      break;

	case kSpacing:
		bandSpacingChanged(GetParam(kSpacing)->Value());
		break;

	case kPitch:
		pitchChanged(GetParam(kPitch)->Value());
		break;

	case kCrush:
		bitCrushChanged( map(GetParam(kCrush)->Value(), 0, 1, Settings::BitCrushMin, Settings::BitCrushMax) );
		break;

	case kDecay:
		decayChanged( map(GetParam(kDecay)->Value(), 0, 1, Settings::DecayMin, Settings::DecayMax) );
		break;

	case kPluckX:
		pluck();
		mPluckX = GetParam(kPluckX)->Value();
		break;

	case kPluckY:
		pluck();
		mPluckY = GetParam(kPluckY)->Value();
		break;

    default:
      break;
  }
}

void IPlugEffect::pluck()
{
	if (mPluckX != -1 && mPluckY != -1)
	{
		for (int b = Settings::BandOffset; b < Settings::BandLast; b += Settings::BandSpacing)
		{
			float normBand = map(b, Settings::BandOffset, Settings::BandLast, 0, 1);
			if (fabs(normBand - GetParam(kPluckX)->Value() / 100.) < 0.01)
			{
				float mag = kMaxSpectralAmp;
				float normY = GetParam(kPluckY)->Value() / 100.;
				float ps = map(normY, 0, 1, M_PI / 64, M_PI / 128);
				specGen.pluck(b, mag, ps);
			}
		}
	}
}