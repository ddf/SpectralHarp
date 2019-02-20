#pragma once

#include "IControl.h"

class TextBox : public ICaptionControl
{
public:
	TextBox(IRECT pR, int paramIdx, const IText& pText, IGraphics* pGraphics, const char * maxText, bool showParamUnits, float scrollSpeed);

	void Draw(IGraphics& pGraphics) override;
	void OnMouseDown(float x, float y, const IMouseMod& pMod) override;
	void OnMouseWheel(float x, float y, const IMouseMod& pMod, float d) override;


	virtual void GrayOut(bool gray) override;

  void CreateContextMenu(IPopupMenu& contextMenu) override;
  void OnContextSelection(int itemSelected) override;

private:
	bool  mShowParamUnits;
	float mScrollSpeed;
	IRECT mTextRect;
};
