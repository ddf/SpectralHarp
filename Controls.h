//
//  Controls.h
//  SpectralHarp
//
//  Created by Damien Quartz on 2/16/18.
//

#ifndef Controls_h
#define Controls_h

#include "IControl.h"

class KnobLineCoronaControl : public IKnobLineControl
{
public:
	KnobLineCoronaControl(IPlugBase* pPlug, IRECT pR, int paramIdx,
						  const IColor* pLineColor, const IColor* pCoronaColor,
						  float coronaThickness, double innerRadius = 0.0, double outerRadius = 0.0,
						  double minAngle = -0.75 * PI, double maxAngle = 0.75 * PI,
						  EDirection direction = kVertical, double gearing = DEFAULT_GEARING);
	
	bool Draw(IGraphics* pGraphics) override;

#if SA_API
	void OnMouseDown(int x, int y, IMouseMod* pMod) override;
#endif

	void OnMouseDrag(int x, int y, int dX, int dY, IMouseMod* pMod) override;

private:
	IColor        mCoronaColor;
	IChannelBlend mCoronaBlend;
};

#endif /* Controls_h */
