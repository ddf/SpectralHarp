#include "TextBox.h"
#include "MidiMapper.h"

TextBox::TextBox(IRECT pR, int paramIdx, const IText& pText, IGraphics* pGraphics, const char * maxText, bool showParamUnits, float scrollSpeed)
	: ICaptionControl(pR, paramIdx, pText, showParamUnits)
	, mTextRect(pR)
	, mScrollSpeed(scrollSpeed)
{
  mTextRect = pR.GetPadded(-5, -2, -5, -2);
	SetTextEntryLength((int)strlen(maxText) - 1);
}

void TextBox::Draw(IGraphics& pGraphics)
{
	pGraphics.FillRect(mText.mTextEntryBGColor, mRECT);
	IColor& borderColor = mText.mTextEntryFGColor;
	const float bi = 2;
	const float bt = mRECT.T;
	const float bb = mRECT.B - 1;
	// bracket on left side
	pGraphics.DrawLine(borderColor, mRECT.L, bt, mRECT.L, bb);
	pGraphics.DrawLine(borderColor, mRECT.L, bt, mRECT.L + bi, bt);
	pGraphics.DrawLine(borderColor, mRECT.L, bb, mRECT.L + bi, bb);
	// bracket on right side
	pGraphics.DrawLine(borderColor, mRECT.R, bt, mRECT.R, bb);
	pGraphics.DrawLine(borderColor, mRECT.R, bt, mRECT.R - bi, bt);
	pGraphics.DrawLine(borderColor, mRECT.R, bb, mRECT.R - bi, bb);

  // debugging to see exactly where the text rect is
  //pGraphics.FillRect(COLOR_GREEN, mTextRect);

	IRECT ourRect = mRECT;
	mRECT = mTextRect;
	if (IsDisabled())
	{
		GetParam()->GetDisplayForHost(GetValue(), true, mStr, false);
		ITextControl::Draw(pGraphics);
	}
	else
	{
		ICaptionControl::Draw(pGraphics);
	}
	mRECT = ourRect;
}

void TextBox::OnMouseDown(float x, float y, const IMouseMod& pMod)
{
	if (pMod.L)
	{
    PromptUserInput(mTextRect);
	}
}

void TextBox::OnMouseWheel(float x, float y, const IMouseMod& pMod, float d)
{
  double value = GetValue();
#ifdef PROTOOLS
	if (pMod->C)
	{
		value += GetParam()->GetStep() * mScrollSpeed/10 * d;
	}
#else
	if (pMod.C || pMod.S)
	{
		value += GetParam()->GetStep() * mScrollSpeed/10 * d;
	}
#endif
	else
	{
		value += GetParam()->GetStep() * mScrollSpeed * d;
	}

  SetValue(value);
	SetDirty();
}

void TextBox::SetDisabled(bool disabled)
{
	ICaptionControl::SetDisabled(disabled);

  mText.mFGColor.A = disabled ? 128 : 255;
}

void TextBox::CreateContextMenu(IPopupMenu& contextMenu)
{
  MidiMapper* mapper = dynamic_cast<MidiMapper*>(GetUI()->GetControlWithTag(kMidiMapper));
  if (mapper != nullptr)
  {
    mapper->CreateContextMenu(contextMenu, GetParamIdx());
  }
}

void TextBox::OnContextSelection(int itemSelected)
{
  MidiMapper* mapper = dynamic_cast<MidiMapper*>(GetUI()->GetControlWithTag(kMidiMapper));
  if (mapper != nullptr)
  {
    mapper->OnContextSelection(itemSelected);
  }
}
