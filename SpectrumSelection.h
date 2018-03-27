#pragma once
#include "IControl.h"

class SpectrumSelection : public IControl
{
public:
	SpectrumSelection(IPlugBase* pPlug, IRECT rect, int bandLowParam, int bandHighParam, IColor back, IColor select, IColor handle);
	~SpectrumSelection();

	virtual void OnMouseDown(int x, int y, IMouseMod* pMod) override;
	virtual void OnMouseDrag(int x, int y, int dX, int dY, IMouseMod* pMod) override;
	virtual void OnMouseUp(int x, int y, IMouseMod* pMod) override;

	virtual bool Draw(IGraphics* pGraphics) override;
	
	void SetAuxParamValueFromPlug(int auxParamIdx, double value) override;

private:

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
};

class SpectrumArrows : public IControl
{
public:
	SpectrumArrows(IPlugBase* pPlug, IRECT rect, IColor color) : IControl(pPlug, rect), mColor(color) {}

	bool Draw(IGraphics* pGraphics) override;

private:
	IColor mColor;
};
