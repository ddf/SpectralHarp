#include "SpectrumSelection.h"

extern float map(float value, float istart, float istop, float ostart, float ostop);

SpectrumSelection::SpectrumSelection(IPlugBase* pPlug, IRECT rect, int bandLowParam, int bandHighParam, IColor back, IColor select, IColor handle) 
	: IControl(pPlug, rect)
	, backgroundColor(back)
	, selectedColor(select)
	, handleColor(handle)
	, handleWidth(6)
	, dragParam(-1)
{
	AddAuxParam(bandLowParam);
	AddAuxParam(bandHighParam);

	const int handT = rect.T;
	const int handB = rect.B;
	handles[0] = IRECT(rect.L, handT, rect.L + handleWidth, handB);
	handles[1] = IRECT(rect.R - handleWidth, handT, rect.R, handB);
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
		int hw = handleWidth / 2;
		AuxParam* param = GetAuxParam(dragParam);
		param->mValue = map(mw, mRECT.L + hw, mRECT.R - hw, 0, 1);
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
