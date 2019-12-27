#pragma once
#include "IControl.h"

using namespace iplug;
using namespace igraphics;

class SpectrumSelection : public IControl
{
public:
	SpectrumSelection(IRECT rect, int lowParamIdx, int highParamIdx, IColor back, IColor select, IColor handle);
	~SpectrumSelection();

	virtual void OnMouseDown(float x, float y, const IMouseMod& pMod) override;
	virtual void OnMouseDrag(float x, float y, float dX, float dY, const IMouseMod& pMod) override;
	virtual void OnMouseUp(float x, float y, const IMouseMod& pMod) override;

  void CreateContextMenu(IPopupMenu& contextMenu) override;
  void OnContextSelection(int itemSelected) override;
  virtual void Draw(IGraphics& pGraphics) override;
	
  void SetValueFromDelegate(double value, int valIdx /* = 0 */) override;

private:
	void DrawHandle(IGraphics& pGraphics, const IRECT& handle);
	void SetParamFromHandle(const int paramIdx);
	void SetHandleFromParam(const int paramIdx);

	IColor backgroundColor;
	IColor selectedColor;
	IColor handleColor;

	IRECT handles[3];
	const int handleWidth;

	// the param whose value we should change
	enum DragParam
	{
		kDragNone = -1,
		kDragLeft,
		kDragRight,
		kDragBoth
	} dragParam;

	// the min x-coord we can drag to
	int    dragMinX;
	// the max x-coord we can drag to
	int    dragMaxX;

  // param index we use in CreateContextMenu
  int contextParam;
};

class SpectrumArrows : public IControl
{
public:
	SpectrumArrows(IRECT rect, IColor color) : IControl(rect), mColor(color) {}

	void Draw(IGraphics& pGraphics) override;

private:
	IColor mColor;
};
