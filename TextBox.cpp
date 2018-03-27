#include "TextBox.h"
#include "SpectralHarp.h"

TextBox::TextBox(IPlugBase* pPlug, IRECT pR, int paramIdx, IText* pText, IGraphics* pGraphics, const char * maxText, bool showParamUnits, float scrollSpeed)
	: ICaptionControl(pPlug, pR, paramIdx, pText, showParamUnits)
	, mTextRect(pR)
	, mScrollSpeed(scrollSpeed)
{
	mTextRect = pR.GetHPadded(-3);
	pGraphics->MeasureIText(pText, const_cast<char*>(maxText), &mTextRect);
#ifdef OS_OSX
	mTextRect.B -= 4;
#endif
	const int offset = (mRECT.H() - mTextRect.H()) / 2;
	mTextRect.T += offset;
	mTextRect.B += offset;

	SetTextEntryLength(strlen(maxText) - 1);
}

bool TextBox::Draw(IGraphics* pGraphics)
{
	pGraphics->FillIRect(&mText.mTextEntryBGColor, &mRECT);
	IColor* borderColor = &mText.mTextEntryFGColor;
	const int bi = 2;
	const int bt = mRECT.T;
	const int bb = mRECT.B - 1;
	// bracket on left side
	pGraphics->DrawLine(borderColor, mRECT.L, bt, mRECT.L, bb);
	pGraphics->DrawLine(borderColor, mRECT.L, bt, mRECT.L + bi, bt);
	pGraphics->DrawLine(borderColor, mRECT.L, bb, mRECT.L + bi, bb);
	// bracket on right side
	pGraphics->DrawLine(borderColor, mRECT.R, bt, mRECT.R, bb);
	pGraphics->DrawLine(borderColor, mRECT.R, bt, mRECT.R - bi, bt);
	pGraphics->DrawLine(borderColor, mRECT.R, bb, mRECT.R - bi, bb);

	IRECT ourRect = mRECT;
	mRECT = mTextRect;
	if (IsGrayed())
	{
		char display[32];
		GetParam()->GetDisplayForHost(mValue, true, display, false);
		mStr.Set(display);
		ITextControl::Draw(pGraphics);
	}
	else
	{
		ICaptionControl::Draw(pGraphics);
	}
	mRECT = ourRect;

	return true;
}

void TextBox::OnMouseDown(int x, int y, IMouseMod* pMod)
{
	if (pMod->L)
	{
		IText ourText = mText;
		IRECT promptRect = mTextRect;
#if defined(OS_OSX)
		mText.mSize -= 2;
		promptRect.T -= 1;
#endif
		mPlug->GetGUI()->PromptUserInput(this, mPlug->GetParam(mParamIdx), &promptRect);
		mText = ourText;
		Redraw();
	}
	else if (pMod->R)
	{
		SpectralHarp* plug = static_cast<SpectralHarp*>(mPlug);
		if (plug != nullptr)
		{
			plug->BeginMIDILearn(mParamIdx, -1, x, y);
		}
	}
}

void TextBox::OnMouseWheel(int x, int y, IMouseMod* pMod, int d)
{
#ifdef PROTOOLS
	if (pMod->C)
	{
		mValue += GetParam()->GetStep() * mScrollSpeed/10 * d;
	}
#else
	if (pMod->C || pMod->S)
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

	mText.mColor.A = gray ? 128 : 255;	
}
