#pragma once
#include "IControl.h"

class SpectrumSelection;

class SpectrumHandle : public IControl
{
public:
  SpectrumHandle(int paramId) : IControl(IRECT(), paramId), mParent(nullptr) {}

  void SetParent(SpectrumSelection* parent) { mParent = parent; }

  virtual void Draw(IGraphics& g) override;
  virtual void SetValueFromDelegate(double value) override;

private:
  SpectrumSelection* mParent;
};

class SpectrumSelection : public IControl
{
public:
	SpectrumSelection(IRECT rect, SpectrumHandle* lowHandle, SpectrumHandle* highHandle, IColor back, IColor select, IColor handle);
	~SpectrumSelection();

	virtual void OnMouseDown(float x, float y, const IMouseMod& pMod) override;
	virtual void OnMouseDrag(float x, float y, float dX, float dY, const IMouseMod& pMod) override;
	virtual void OnMouseUp(float x, float y, const IMouseMod& pMod) override;

	virtual void Draw(IGraphics& pGraphics) override;
	
	void SetAuxParamValueFromPlug(int auxParamIdx, double value);

private:
	void DrawHandle(IGraphics& pGraphics, const IRECT& handle);
	void SetParamFromHandle(const int paramIdx);
	void SetHandleFromParam(const int paramIdx);

  SpectrumHandle* mHandles[2];

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
	SpectrumArrows(IRECT rect, IColor color) : IControl(rect), mColor(color) {}

	void Draw(IGraphics& pGraphics) override;

private:
	IColor mColor;
};
