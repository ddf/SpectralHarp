#include "StringControl.h"
#include "IPlugEffect.h"
#include "src/Settings.h"

extern float map(float value, float istart, float istop, float ostart, float ostop);

const float kPadding = 5;

bool StringControl::Draw(IGraphics* pGraphics)
{
	for (int b = Settings::BandOffset; b < Settings::BandLast; b += Settings::BandSpacing)
	{
		float x = map(b, Settings::BandOffset, Settings::BandLast-1, mRECT.L + kPadding, mRECT.R - kPadding);
		float p = spectrum.getBandPhase(b) + stringAnimation;
		float m = spectrum.getBandMagnitude(b);

		float br = map(m, 0, kMaxSpectralAmp, 0.4f, 1);
		IColor color(255, 255 * br, 255 * br, 255 * br);

		float w = map(m, 0, kMaxSpectralAmp, 0, 6);
		const float segments = 32;
		const float segLength = mRECT.H() / segments;
		float py0 = 0;
		float px0 = x + w*sinf(p);
		for (int i = 1; i < segments+1; ++i)
		{
			float py1 = i*segLength;
			float s1 = py1 / mRECT.H() * M_PI * 8 + p;
			float px1 = x + w*sinf(s1);
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