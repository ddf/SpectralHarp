#include "KnobLineCoronaControl.h"
#include "MidiMapper.h"

static const double kRadToDeg = 180.0 / PI;

#pragma  region KnobLineCoronaControl
KnobLineCoronaControl::KnobLineCoronaControl(IRECT pR, int paramIdx,
											 const IColor& color, const IColor& coronaColor,
											 float lineThickness,
											 double innerRadius, double outerRadius,
											 double minAngle, double maxAngle,
											 EDirection direction, double gearing)
: IKnobControlBase(pR, paramIdx, direction, gearing)
, mCX(mRECT.MW())
, mCY(mRECT.MH())
, mInnerRadius(innerRadius)
, mOuterRadius(outerRadius)
, mMinAngle(minAngle)
, mMaxAngle(maxAngle)
, mColor(color)
, mCoronaColor(coronaColor)
, mLineThickness(lineThickness)
, mLabelControl(nullptr)
, mSharedLabel(false)
, mHasMouse(false)
{
  if (mOuterRadius == 0.0)
  {
    mOuterRadius = 0.5 * pR.W();
  }
}

void KnobLineCoronaControl::Draw(IGraphics& pGraphics)
{
	float v = mMinAngle + (float)mValue * (mMaxAngle - mMinAngle);
  pGraphics.DrawArc(mCoronaColor, mCX, mCY, mOuterRadius, mMinAngle*kRadToDeg, v*kRadToDeg, 0, mLineThickness);
  IColor color(mCoronaColor.A, mCoronaColor.R / 2, mCoronaColor.G / 2, mCoronaColor.B / 2);
  pGraphics.DrawArc(color, mCX, mCY, mOuterRadius, v*kRadToDeg, mMaxAngle*kRadToDeg, 0, mLineThickness);

	float sinV = (float)sin(v);
	float cosV = (float)cos(v);
	float x1 = mCX + mInnerRadius * sinV, y1 = mCY - mInnerRadius * cosV;
	float x2 = mCX + mOuterRadius * sinV, y2 = mCY - mOuterRadius * cosV;
	return pGraphics.DrawLine(mColor, x1, y1, x2, y2, 0, mLineThickness);
}

void KnobLineCoronaControl::OnMouseDown(float x, float y, const IMouseMod& pMod)
{
  mHasMouse = true;
}

void KnobLineCoronaControl::OnMouseDrag(float x, float y, float dX, float dY, const IMouseMod& pMod)
{
	double gearing = mGearing;
	
#ifdef PROTOOLS
#ifdef OS_WIN
	if (pMod->C) gearing *= 10.0;
#else
	if (pMod->R) gearing *= 10.0;
#endif
#else
	if (pMod.C || pMod.S) gearing *= 10.0;
#endif
	
	mValue += (double)dY / (double)(mRECT.T - mRECT.B) / gearing;
	mValue += (double)dX / (double)(mRECT.R - mRECT.L) / gearing;
	
	SetDirty();
}

void KnobLineCoronaControl::OnMouseUp(float x, float y, const IMouseMod& pMod)
{
	if (!mRECT.Contains(x, y))
	{
		HideLabel();
	}

	mHasMouse = false;
}

void KnobLineCoronaControl::OnMouseOver(float x, float y, const IMouseMod& pMod)
{
	ShowLabel();
}

void KnobLineCoronaControl::OnMouseOut()
{
	if (!mHasMouse)
	{
		HideLabel();
	}
}

void KnobLineCoronaControl::ShowLabel()
{
	if (mLabelControl != nullptr)
	{
		// if our label was hidden when we attached it, 
		// that means we should reposition it below the knob before displaying it.
		if (mSharedLabel)
		{
			IRECT targetRect = mRECT;
			targetRect.T = targetRect.B - 16;
			IRECT labelRect = mLabelControl->GetRECT();
			labelRect = targetRect;
      mLabelControl->SetTargetRECT(targetRect);
		}
		SetValDisplayControl(mLabelControl);
		SetDirty();
	}
}

void KnobLineCoronaControl::HideLabel()
{
	SetValDisplayControl(nullptr);
	if (mLabelControl != nullptr)
	{
		mLabelControl->SetStr(mLabelString.Get());
		SetDirty(false);
	}
}

void KnobLineCoronaControl::SetLabelControl(ITextControl* control, const char * label, bool bShared)
{
	mLabelControl = control;
	mSharedLabel = bShared;
	if (mLabelControl != nullptr)
	{
		mLabelString.Set(label);
	}
	else
	{
		mLabelString.Set("");
		mSharedLabel = false;
	}

	// if our label control is shared, extend our rect to include where we will place the label when showing it.
	// this ensures that the area the label draws in will be cleared when the labels is moved elsewhere.
	if (mSharedLabel)
	{
		mRECT.L -= 15;
		mRECT.R += 15;
		mRECT.B += 15;
	}
}

void KnobLineCoronaControl::CreateContextMenu(IPopupMenu& contextMenu)
{
  MidiMapper* control = dynamic_cast<MidiMapper*>(GetUI()->GetControlWithTag(kMidiMapper));
  if (control != nullptr)
  {
    control->CreateContextMenu(contextMenu, mParamIdx);
  }
}

void KnobLineCoronaControl::OnContextSelection(int itemSelected)
{
  MidiMapper* control = dynamic_cast<MidiMapper*>(GetUI()->GetControlWithTag(kMidiMapper));
  if (control != nullptr)
  {
    control->OnContextSelection(itemSelected);
  }
}

void KnobLineCoronaControl::SetDirty(bool triggerAction /*= true*/)
{
  IKnobControlBase::SetDirty(triggerAction);

  if (triggerAction && mLabelControl != nullptr && mValDisplayControl == mLabelControl)
  {
    const IParam* pParam = GetParam();

    if (pParam)
    {
      WDL_String str;
      pParam->GetDisplayForHost(str);
      str.Append(" ");
      str.Append(pParam->GetLabelForHost());
      mLabelControl->SetStr(str.Get());
    }
  }
}


#pragma  endregion KnobLineCoronaControl
