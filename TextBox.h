#pragma once

#include "IControl.h"

class TextBox : public ICaptionControl
{
public:
	TextBox(IPlugBase* pPlug, IRECT pR, int paramIdx, IText* pText, IGraphics* pGraphics, const char * maxText, bool showParamUnits, float scrollSpeed);

	bool Draw(IGraphics* pGraphics) override;
	void OnMouseDown(int x, int y, IMouseMod* pMod) override;
	void OnMouseWheel(int x, int y, IMouseMod* pMod, int d) override;


	virtual void GrayOut(bool gray) override;

private:
	bool  mShowParamUnits;
	float mScrollSpeed;
	IRECT mTextRect;
};
