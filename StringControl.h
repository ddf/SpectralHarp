#pragma once
#include "IControl.h"

class SpectralGen;

class StringControl : public IControl
{
public:
	StringControl(const SpectralGen& rSpectrum, IPlugBase *pPlug, IRECT pR, int handleRadius, int paramA, int paramB);

	bool Draw(IGraphics*) override;

	void OnMouseDown(int x, int y, IMouseMod* pMod) override;

	void OnMouseUp(int x, int y, IMouseMod* pMod) override;

	void OnMouseDrag(int x, int y, int dX, int dY, IMouseMod* pMod) override;

	void SetDirty(bool pushParamToPlug = true) override;

private:

	void SnapToMouse(int x, int y);

	const SpectralGen& spectrum;
	int mHandleRadius;
	IColor mHandleColor;
	float stringAnimation;
};

