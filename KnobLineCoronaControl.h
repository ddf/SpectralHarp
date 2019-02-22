#pragma  once

#include "IControl.h"
#include "MidiMapper.h"

class KnobLineCoronaControl : public IKnobControlBase
{
public:
	KnobLineCoronaControl(IRECT pR, int paramIdx,
						  const IColor& lineColor, const IColor& coronaColor,
						  float lineThickness = 1.0f, double innerRadius = 0.0, double outerRadius = 0.0,
						  double minAngle = -0.75 * PI, double maxAngle = 0.75 * PI,
						  EDirection direction = kVertical, double gearing = DEFAULT_GEARING);
	
	void Draw(IGraphics& pGraphics) override;

	void OnMouseDown(float x, float y, const IMouseMod& pMod) override;
	void OnMouseDrag(float x, float y, float dX, float dY, const IMouseMod& pMod) override;
	void OnMouseUp(float x, float y, const IMouseMod& pMod) override;

	void OnMouseOver(float x, float y, const IMouseMod& pMod) override;
	void OnMouseOut() override;	

	void SetLabelControl(ITextControl* control, const char * label, bool bShared = false);

  void CreateContextMenu(IPopupMenu& contextMenu) override;
  void OnContextSelection(int itemSelected) override;

  void SetDirty(bool triggerAction = true) override;

private:
	void ShowLabel();
	void HideLabel();
	
	float		  mCX, mCY;
	bool		  mHasMouse;
  IColor    mColor;
	IColor    mCoronaColor;
  float     mLineThickness;

	ITextControl* mLabelControl;
	WDL_String	  mLabelString;
	bool		      mSharedLabel;
  double mInnerRadius;
  double mOuterRadius;
  double mMinAngle;
  double mMaxAngle;
};
