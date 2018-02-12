#include "StringControl.h"
#include "SpectralHarp.h"
#include "src/Settings.h"

const float kPadding = 16;

bool StringControl::Draw(IGraphics* pGraphics)
{
	//const int numBands = (Settings::BandLast - Settings::BandFirst) * Settings::BandDensity;
	const int numBands = Settings::BandDensity;
    if ( numBands > 0 )
    {
        for (int b = 0; b <= numBands; ++b)
        {
            const int bindx = (int)roundf(Settings::Map((float)b, 0, (float)numBands, Settings::BandFirst, Settings::BandLast));
            const float x = Settings::Map((float)b, 0, (float)numBands, mRECT.L + kPadding, mRECT.R - kPadding);
            const float p = spectrum.getBandPhase(bindx) + stringAnimation;
            const float m = spectrum.getBandMagnitude(bindx);

            const int g = (int)(255.f * Settings::Map(m, 0, Settings::SpectralAmpMax, 0.4f, 1.f));
            const IColor color(255, g, g, g);

            const float w = Settings::Map(m, 0, Settings::SpectralAmpMax, 0, 6);
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

void StringControl::SnapToMouse(int x, int y)
{
	const float strumX = Settings::Map(x, mRECT.L + kPadding, mRECT.R - kPadding, 0, 1);
	GetAuxParam(0)->mValue = BOUNDED(strumX, 0, 1);
	GetAuxParam(1)->mValue = BOUNDED((double)y / (double)mRECT.H(), 0, 1);

	SetDirty();
}