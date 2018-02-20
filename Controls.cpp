//
//  Controls.cpp
//  SpectralHarp
//
//  Created by Damien Quartz on 2/16/18.
//

#include "Controls.h"
#include "SpectralHarp.h"

#pragma  region KnobLineCoronaControl
KnobLineCoronaControl::KnobLineCoronaControl(IPlugBase* pPlug, IRECT pR, int paramIdx,
											 const IColor* pColor, const IColor* pCoronaColor,
											 float coronaThickness,
											 double innerRadius, double outerRadius,
											 double minAngle, double maxAngle,
											 EDirection direction, double gearing)
: IKnobLineControl(pPlug, pR, paramIdx, pColor, innerRadius, outerRadius, minAngle, maxAngle, direction, gearing)
, mCoronaColor(*pCoronaColor)
, mCoronaBlend(IChannelBlend::kBlendAdd, coronaThickness)
{
}

bool KnobLineCoronaControl::Draw(IGraphics* pGraphics)
{
	float cx = mRECT.MW(), cy = mRECT.MH();
	float v = mMinAngle + (float)mValue * (mMaxAngle - mMinAngle);
	for (int i = 0; i <= mCoronaBlend.mWeight; ++i)
	{
		IColor color = mCoronaColor;
		pGraphics->DrawArc(&color, cx, cy, mOuterRadius - i, mMinAngle, v, nullptr, true);
		color.R /= 2;
		color.G /= 2;
		color.B /= 2;
		pGraphics->DrawArc(&color, cx, cy, mOuterRadius - i, v, mMaxAngle, nullptr, true);
	}
	return IKnobLineControl::Draw(pGraphics);
}

void KnobLineCoronaControl::OnMouseDown(int x, int y, IMouseMod* pMod)
{
	if (pMod->R)
	{
		SpectralHarp* harp = dynamic_cast<SpectralHarp*>(mPlug);
		if (harp != nullptr)
		{
			harp->BeginMIDILearn(mParamIdx, -1, x, y);
		}
	}
}

void KnobLineCoronaControl::OnMouseDrag(int x, int y, int dX, int dY, IMouseMod* pMod)
{
	double gearing = mGearing;
	
#ifdef PROTOOLS
#ifdef OS_WIN
	if (pMod->C) gearing *= 10.0;
#else
	if (pMod->R) gearing *= 10.0;
#endif
#else
	if (pMod->C || pMod->S) gearing *= 10.0;
#endif
	
	mValue += (double)dY / (double)(mRECT.T - mRECT.B) / gearing;
	mValue += (double)dX / (double)(mRECT.R - mRECT.L) / gearing;
	
	SetDirty();
}

#pragma  endregion KnobLineCoronaControl
