#include "SpectralHarp.h"
#include "TextBox.h"

TextBox::TextBox(IRECT pR, int paramIdx, const IText& pText, IGraphics* pGraphics, const char * maxText, bool showParamUnits, float scrollSpeed)
	: ICaptionControl(pR, paramIdx, pText, showParamUnits)
	, mTextRect(pR)
	, mScrollSpeed(scrollSpeed)
{
  mTextRect = pR.GetPadded(2, 1, -2, -1);
  //pGraphics->MeasureText(pText, maxText, mTextRect);
#ifdef OS_MAC
  mTextRect.B -= 1;
#endif
//  const int offset = (mRECT.H() - mTextRect.H()) / 2;
//  mTextRect.T += offset;
//  mTextRect.B += offset;

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

	IRECT ourRect = mRECT;
	mRECT = mTextRect;
	if (IsGrayed())
	{
		GetParam()->GetDisplayForHost(mValue, true, mStr, false);
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
    // IGraphicsMac scales text size by 0.75 when creating a native text entry,
    // but when rendering with Cairo this causes the text entry text to be smaller than the rendered text.
    // So we increase text size by the that ratio.
    // Presumably this happens because Cairo renders text with Quartz, which is an Apple-provided API
#ifdef OS_MAC
    mText.mSize = (int)round(mText.mSize*1.33333);
#endif
    // Platform text entry display does not take into account draw scale, so we have to do that here.
    mText.mSize = (int)round(mText.mSize*GetUI()->GetDrawScale());
    PromptUserInput(promptRect);
		mText = ourText;
	}
	else if (pMod.R)
	{
		SpectralHarp* plug = static_cast<SpectralHarp*>(GetDelegate());
		if (plug != nullptr)
		{
      for (int i = 0; i < GetUI()->NControls(); ++i)
      {
        if (GetUI()->GetControl(i) == this)
        {
          plug->BeginMIDILearn(i, mParamIdx, -1, x, y);
          break;
        }
      }
		}
	}
}

void TextBox::OnMouseWheel(float x, float y, const IMouseMod& pMod, float d)
{
#ifdef PROTOOLS
	if (pMod->C)
	{
		mValue += GetParam()->GetStep() * mScrollSpeed/10 * d;
	}
#else
	if (pMod.C || pMod.S)
	{
		mValue += GetParam()->GetStep() * mScrollSpeed/10 * d;
	}
#endif
	else
	{
		mValue += GetParam()->GetStep() * mScrollSpeed * d;
	}

	SetDirty();
}

void TextBox::GrayOut(bool gray)
{
	ICaptionControl::GrayOut(gray);

  mText.mFGColor.A = gray ? 128 : 255;
}
