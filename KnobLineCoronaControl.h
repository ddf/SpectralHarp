#pragma  once

#include "IControl.h"

class KnobLineCoronaControl : public IKnobLineControl
{
public:
	KnobLineCoronaControl(IPlugBase* pPlug, IRECT pR, int paramIdx,
						  const IColor* pLineColor, const IColor* pCoronaColor,
						  float coronaThickness = 0.0f, double innerRadius = 0.0, double outerRadius = 0.0,
						  double minAngle = -0.75 * PI, double maxAngle = 0.75 * PI,
						  EDirection direction = kVertical, double gearing = DEFAULT_GEARING);
	
	bool Draw(IGraphics* pGraphics) override;

	void OnMouseDown(int x, int y, IMouseMod* pMod) override;
	void OnMouseDrag(int x, int y, int dX, int dY, IMouseMod* pMod) override;
	void OnMouseUp(int x, int y, IMouseMod* pMod) override;

	void OnMouseOver(int x, int y, IMouseMod* pMod) override;
	void OnMouseOut() override;	

	void SetLabelControl(ITextControl* control, bool bShared = false);

private:
	void ShowLabel();
	void HideLabel();
	
	float		  mCX, mCY;
	bool		  mHasMouse;
	IColor        mCoronaColor;
	IChannelBlend mCoronaBlend;
	ITextControl* mLabelControl;
	WDL_String	  mLabelString;
	bool		  mSharedLabel;
};
