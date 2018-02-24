#include "StringControl.h"
#include "SpectralGen.h"
#include "SpectralHarp.h"
#include "Params.h"

const float kPadding = 16;

StringControl::StringControl(const SpectralGen& rSpectrum, IPlugBase *pPlug, IRECT pR, int handleRadius, int paramA, int paramB) 
	: IControl(pPlug, pR)
	, spectrum(rSpectrum)
	, mHandleRadius(handleRadius)
	, mHandleColor(COLOR_WHITE)
	, stringAnimation(0)
{
	AddAuxParam(paramA);
	AddAuxParam(paramB);
}

bool StringControl::Draw(IGraphics* pGraphics)
{
	//const int numBands = (Settings::BandLast - Settings::BandFirst) * Settings::BandDensity;
	const float numBands = (float)mPlug->GetParam(kBandDensity)->Int();
	const float bandFirst = (float)mPlug->GetParam(kBandFirst)->Int();
	const float bandLast = (float)mPlug->GetParam(kBandLast)->Int();
    if ( numBands > 0 )
    {
        for (int b = 0; b <= numBands; ++b)
        {
            const float freq = roundf(Map((float)b, 0, numBands, bandFirst, bandLast));
            const float x = Map((float)b, 0, numBands, mRECT.L + kPadding, mRECT.R - kPadding);
            const float p = spectrum.getBandPhase(freq) + stringAnimation;
            const float m = spectrum.getBandMagnitude(freq);

            const int g = (int)(255.f * Map(m, 0, kSpectralAmpMax, 0.4f, 1.f));
            const IColor color(255, g, g, g);

            const float w = Map(m, 0, kSpectralAmpMax, 0, 6);
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

void StringControl::OnMouseDown(int x, int y, IMouseMod* pMod)
{
	if ( pMod->R )
	{
		SpectralHarp* harp = dynamic_cast<SpectralHarp*>(mPlug);
		if (harp != nullptr)
		{
			harp->BeginMIDILearn(GetAuxParam(0)->mParamIdx, GetAuxParam(1)->mParamIdx, x, y);
		}
	}
	else if ( pMod->L )
	{
		mHandleColor = COLOR_BLACK;
		SnapToMouse(x, y);
	}
}

void StringControl::OnMouseUp(int x, int y, IMouseMod* pMod)
{
	if (pMod->L)
	{
		mHandleColor = COLOR_WHITE;
	}
}

void StringControl::OnMouseDrag(int x, int y, int dX, int dY, IMouseMod* pMod)
{
	if (pMod->L)
	{
		SnapToMouse(x, y);
	}
}

void StringControl::SetDirty(bool pushParamToPlug /*= true*/)
{
	mDirty = true;

	if (pushParamToPlug && mPlug)
	{
		SetAllAuxParamsFromGUI();
	}
}

void StringControl::SnapToMouse(int x, int y)
{
	const float strumX = Map((float)x, mRECT.L + kPadding, mRECT.R - kPadding, 0, 1);
	const float strumY = Map((float)y, mRECT.T, mRECT.B, 1, 0);
	GetAuxParam(0)->mValue = BOUNDED(strumX, 0, 1);
	GetAuxParam(1)->mValue = BOUNDED(strumY, 0, 1);

	SetDirty();
}
