#pragma once
#include "IControl.h"
#include "src/SpectralGen.h"

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

	//  void OnMouseOver(int x, int y, IMouseMod* pMod)
	//  {
	//    return SnapToMouse(x, y);
	//  }
	//  
	void OnMouseDown(int x, int y, IMouseMod* pMod)
	{
		mHandleColor = COLOR_BLACK;
		return SnapToMouse(x, y);
	}

	void OnMouseUp(int x, int y, IMouseMod* pMod)
	{
		mHandleColor = COLOR_WHITE;
	}

	void OnMouseDrag(int x, int y, int dX, int dY, IMouseMod* pMod)
	{
		return SnapToMouse(x, y);
	}

	void SnapToMouse(int x, int y)
	{
		GetAuxParam(0)->mValue = BOUNDED((double)x / (double)mRECT.W(), 0, 1);
		GetAuxParam(1)->mValue = BOUNDED((double)y / (double)mRECT.H(), 0, 1);

		SetDirty();
	}

	void SetDirty(bool pushParamToPlug = true)
	{
		mDirty = true;

		if (pushParamToPlug && mPlug)
		{
			SetAllAuxParamsFromGUI();
		}
	}
private:
	const SpectralGen& spectrum;
	int mHandleRadius;
	IColor mHandleColor;
	float stringAnimation;
};

