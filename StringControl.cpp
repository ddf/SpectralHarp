#include "StringControl.h"
#include "IPlugEffect.h"
#include "src/Settings.h"

extern float map(float value, float istart, float istop, float ostart, float ostop);

const float kPadding = 5;

bool StringControl::Draw(IGraphics* pGraphics)
{
	const int numBands = (Settings::BandLast - Settings::BandFirst) * Settings::BandDensity;
	for (float b = 0; b <= 1; b+=bandStep)
	{
		const int bindx = (int)roundf(map(b, 0, 1, Settings::BandFirst, Settings::BandLast));
		const float x = map(b, 0, 1, mRECT.L + kPadding, mRECT.R - kPadding);
		const float p = spectrum.getBandPhase(bindx) + stringAnimation;
		const float m = spectrum.getBandMagnitude(bindx);

		const int g = (int)(255.f * map(m, 0, kMaxSpectralAmp, 0.4f, 1.f));
		const IColor color(255, g, g, g);

		const float w = map(m, 0, kMaxSpectralAmp, 0, 6);
		const float segments = 32;
		const float segLength = mRECT.H() / segments;
		float py0 = 0;
		float px0 = x + w*sinf(p);
		for (int i = 1; i < segments+1; ++i)
		{
			const float py1 = i*segLength;
			const float s1 = py1 / mRECT.H() * (float)M_PI * 8 + p;
			const float px1 = x + w*sinf(s1);
			pGraphics->DrawLine(&color, px0, py0, px1, py1);
			px0 = px1;
			py0 = py1;
		}
	}

	const float dt = 1.0f / 60.f;

	stringAnimation += dt * TWO_PI * 2;
	if (stringAnimation > TWO_PI)
	{
		stringAnimation -= TWO_PI;
	}

	//double xpos = GetAuxParam(0)->mValue * mRECT.W();
	//double ypos = GetAuxParam(1)->mValue * mRECT.H();

	//pGraphics->DrawLine(&mHandleColor, xpos + mRECT.L, mRECT.T, xpos + mRECT.L, mRECT.B, 0, false);
	//pGraphics->DrawLine(&mHandleColor, mRECT.L, ypos + mRECT.T, mRECT.R, ypos + mRECT.T, 0, false);
	//pGraphics->FillCircle(&mHandleColor, xpos + mRECT.L, ypos + mRECT.T, mHandleRadius, 0, true);

	Redraw();

	return true;
}