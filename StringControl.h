#pragma once
#include "IControl.h"

class SpectralGen;

class StringControl : public IControl
{
public:
	StringControl(const SpectralGen& rSpectrum, IPlugBase *pPlug, IRECT pR, int handleRadius, int paramA, int paramB)
		: IControl(pPlug, pR)
		, spectrum(rSpectrum)
		, mHandleRadius(handleRadius)
		, mHandleColor(COLOR_WHITE)
		, stringAnimation(0)
	{
		AddAuxParam(paramA);
		AddAuxParam(paramB);
	}

	bool Draw(IGraphics*);

	void OnMouseDown(int x, int y, IMouseMod* pMod) override;

	void OnMouseUp(int x, int y, IMouseMod* pMod) override
	{
		mHandleColor = COLOR_WHITE;
	}

	void OnMouseDrag(int x, int y, int dX, int dY, IMouseMod* pMod) override
	{
		SnapToMouse(x, y);
	}

	void SetDirty(bool pushParamToPlug = true) override
	{
		mDirty = true;

		if (pushParamToPlug && mPlug)
		{
			SetAllAuxParamsFromGUI();
		}
	}

private:

	void SnapToMouse(int x, int y);

	const SpectralGen& spectrum;
	int mHandleRadius;
	IColor mHandleColor;
	float stringAnimation;
};

