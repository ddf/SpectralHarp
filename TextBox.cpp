#include "TextBox.h"
#include "MidiMapper.h"

TextBox::TextBox(IRECT pR, int paramIdx, const IText& pText, IGraphics* pGraphics, const char * maxText, bool showParamUnits, float scrollSpeed)
	: ICaptionControl(pR, paramIdx, pText, showParamUnits)
	, mTextRect(pR)
	, mScrollSpeed(scrollSpeed)
{
  mTextRect = pR.GetPadded(-5, -2, -5, -2);
  // windows native text edits don't do vertical alignment
  // so we have to clamp the text rect to the height of the text
  // and center it manually so that our text and native text are vertically aligned
#ifdef OS_WIN
  IRECT mr;
  pGraphics->MeasureText(pText, maxText, mr);
  const int offset = (mTextRect.H() - mr.H()) / 2;
  mTextRect.T += offset;
  mTextRect.B -= offset;
#endif

	SetTextEntryLength((int)strlen(maxText) - 1);
}

void TextBox::Draw(IGraphics& pGraphics)
{
	pGraphics.FillRect(mText.mTextEntryBGColor, mRECT);
	IColor& borderColor = mText.mTextEntryFGColor;
	const int bi = 2;
	const int bt = mRECT.T;
	const int bb = mRECT.B - 1;
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
		IText ourText = mText;
		IRECT promptRect = mTextRect;
    double textSize = mText.mSize;
    // Platform text entry display does not take into account draw scale, so we have to do that here.
    mText.mSize = (int)round(textSize*GetUI()->GetDrawScale());
    PromptUserInput(promptRect);
		mText = ourText;
	}
}

void TextBox::OnMouseWheel(float x, float y, const IMouseMod& pMod, float d)
{
  float value = GetValue();
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
