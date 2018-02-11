#include "SpectrumSelection.h"

extern float map(float value, float istart, float istop, float ostart, float ostop);

SpectrumSelection::SpectrumSelection(IPlugBase* pPlug, IRECT rect, int bandLowParam, int bandHighParam, IColor back, IColor select, IColor handle) 
	: IControl(pPlug, rect)
	, backgroundColor(back)
	, selectedColor(select)
	, handleColor(handle)
	, dragParam(-1)
{
	AddAuxParam(bandHighParam);
	AddAuxParam(bandLowParam);

	handles[0] = rect.SubRectHorizontal(35, 0);
	handles[1] = rect.SubRectHorizontal(35, 34);
	handleWidth = handles[0].W();
}

SpectrumSelection::~SpectrumSelection()
{
}

void SpectrumSelection::OnMouseDown(int x, int y, IMouseMod* pMod)
{
	if (handles[0].Contains(x, y))
	{
		dragParam = 0;
		dragMinX = mRECT.L;
		dragMaxX = handles[1].L;
	}
	else if (handles[1].Contains(x, y))
	{
		dragParam = 1;
		dragMinX = handles[0].R;
		dragMaxX = mRECT.R;
	}
}

void SpectrumSelection::OnMouseDrag(int x, int y, int dX, int dY, IMouseMod* pMod)
{
	if (dragParam != -1) 
	{
		IRECT& handle = handles[dragParam];
		if (dX > 0)
		{
			if (handle.R + dX > dragMaxX)
			{
				handle.R = dragMaxX;
			}
			else
			{
				handle.R += dX;
			}

			handle.L = handle.R - handleWidth;
		}
		else
		{
			if (handle.L + dX < dragMinX)
			{
				handle.L = dragMinX;
			}
			else
			{
				handle.L += dX;
			}

			handle.R = handle.L + handleWidth;
		}
		float mw = handle.MW();
		AuxParam* param = GetAuxParam(dragParam);
		param->mValue = map(mw, mRECT.L + mw, mRECT.R - mw, 0, 1);
		mPlug->SetParameterFromGUI(param->mParamIdx, param->mValue);
		SetDirty(false);
	}
}

void SpectrumSelection::OnMouseUp(int x, int y, IMouseMod* pMod)
{
	dragParam = -1;
}

bool SpectrumSelection::Draw(IGraphics* pGraphics)
{
	IRECT backRect = mRECT;
	backRect.L += handleWidth / 2;
	backRect.R -= handleWidth / 2;
	pGraphics->FillIRect(&backgroundColor, &backRect);

	IRECT selectedRect = mRECT;
	selectedRect.L = handles[0].MW();
	selectedRect.R = handles[1].MW();
	pGraphics->FillIRect(&selectedColor, &selectedRect);

	pGraphics->FillIRect(&handleColor, &handles[0]);
	pGraphics->FillIRect(&handleColor, &handles[1]);

	return true;
}
