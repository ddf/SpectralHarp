#pragma once
#include "IControl.h"

class SpectralGen;

class StringControl : public IControl
{
public:
	StringControl(const SpectralGen& rSpectrum, IRECT pR, int handleRadius);

	void Draw(IGraphics&) override;

	void OnMouseDown(float x, float y, const IMouseMod& pMod) override;

	void OnMouseUp(float x, float y, const IMouseMod& pMod) override;

	void OnMouseDrag(float x, float y, float dX, float dY, const IMouseMod& pMod) override;

private:

	void SnapToMouse(float x, float y);

	const SpectralGen& spectrum;
	int mHandleRadius;
	float mMouseX;
	float mMouseY;
	float stringAnimation;
};

