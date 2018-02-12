#include "SpectrumSelection.h"
#include "src/Settings.h"

SpectrumSelection::SpectrumSelection(IPlugBase* pPlug, IRECT rect, int bandLowParam, int bandHighParam, IColor back, IColor select, IColor handle) 
	: IControl(pPlug, rect)
	, backgroundColor(back)
	, selectedColor(select)
	, handleColor(handle)
	, handleWidth(6)
	, dragParam(kDragNone)
{
	AddAuxParam(bandLowParam);
	AddAuxParam(bandHighParam);

	const int handT = rect.T;
	const int handB = rect.B;
	handles[kDragLeft] = IRECT(rect.L, handT, rect.L + handleWidth, handB);
	handles[kDragRight] = IRECT(rect.R - handleWidth, handT, rect.R, handB);
	handles[kDragBoth] = IRECT(handles[0].MW(), handT, handles[1].MW(), handB);
}

SpectrumSelection::~SpectrumSelection()
{
}

void SpectrumSelection::OnMouseDown(int x, int y, IMouseMod* pMod)
{
	if (handles[kDragLeft].Contains(x, y))
	{
		dragParam = kDragLeft;
		dragMinX = mRECT.L;
		dragMaxX = handles[kDragRight].L - handleWidth;
	}
	else if (handles[kDragRight].Contains(x, y))
	{
		dragParam = kDragRight;
		dragMinX = handles[kDragLeft].R + handleWidth;
		dragMaxX = mRECT.R;
	}
	else if (handles[kDragBoth].Contains(x, y))
	{
		dragParam = kDragBoth;
		dragMinX = mRECT.L + handleWidth / 2;
		dragMaxX = mRECT.R - handleWidth / 2;
	}
}

void SpectrumSelection::OnMouseDrag(int x, int y, int dX, int dY, IMouseMod* pMod)
{
	if (dragParam != kDragNone) 
	{
		IRECT& handle = handles[dragParam];
		const int width = handle.W();
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

			handle.L = handle.R - width;
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

			handle.R = handle.L + width;
		}

		switch (dragParam)
		{
		case kDragLeft:
			handles[kDragBoth].L = handle.MW();
			SetParamFromHandle(kDragLeft);
			break;

		case kDragRight:
			handles[kDragBoth].R = handle.MW();
			SetParamFromHandle(kDragRight);
			break;

		case kDragBoth:
			handles[kDragLeft].L = handle.L - handleWidth / 2;
			handles[kDragLeft].R = handle.L + handleWidth / 2;
			handles[kDragRight].L = handle.R - handleWidth / 2;
			handles[kDragRight].R = handle.R + handleWidth / 2;
			SetParamFromHandle(kDragLeft);
			SetParamFromHandle(kDragRight);
			break;
		}

		SetDirty(false);
	}
}

void SpectrumSelection::OnMouseUp(int x, int y, IMouseMod* pMod)
{
	dragParam = kDragNone;
}

bool SpectrumSelection::Draw(IGraphics* pGraphics)
{
	IRECT backRect = mRECT;
	backRect.L += handleWidth / 2;
	backRect.R -= handleWidth / 2;
	pGraphics->FillIRect(&backgroundColor, &backRect);

	pGraphics->FillIRect(&selectedColor, &handles[kDragBoth]);

	pGraphics->FillIRect(&handleColor, &handles[kDragLeft]);
	pGraphics->FillIRect(&handleColor, &handles[kDragRight]);

	return true;
}

void SpectrumSelection::SetParamFromHandle(const int paramIdx)
{
	float mw = handles[paramIdx].MW();
	int hw = handleWidth / 2;
	AuxParam* param = GetAuxParam(paramIdx);
	param->mValue = Settings::Map(mw, mRECT.L + hw, mRECT.R - hw, 0, 1);
	mPlug->SetParameterFromGUI(param->mParamIdx, param->mValue);
}