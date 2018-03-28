#include "SpectrumSelection.h"
#include "SpectralHarp.h"
#include "Params.h"

SpectrumSelection::SpectrumSelection(IPlugBase* pPlug, IRECT rect, int bandLowParam, int bandHighParam, IColor back, IColor select, IColor handle) 
	: IControl(pPlug, rect)
	, backgroundColor(back)
	, selectedColor(select)
	, handleColor(handle)
	, handleWidth(16)
	, dragParam(kDragNone)
{
	AddAuxParam(bandLowParam);
	AddAuxParam(bandHighParam);

	const int handT = rect.T;
	const int handB = rect.B;
	handles[kDragLeft] = IRECT(rect.L, handT, rect.L + handleWidth, handB);
	handles[kDragRight] = IRECT(rect.R - handleWidth, handT, rect.R, handB);
	handles[kDragBoth] = IRECT((int)handles[0].L, handT, (int)handles[1].R, handB);
}

SpectrumSelection::~SpectrumSelection()
{
}

void SpectrumSelection::OnMouseDown(int x, int y, IMouseMod* pMod)
{
	if (handles[kDragLeft].Contains(x, y))
	{
		if (pMod->R)
		{
			SpectralHarp* harp = dynamic_cast<SpectralHarp*>(mPlug);
			if (harp != nullptr)
			{
				harp->BeginMIDILearn(GetAuxParam(kDragLeft)->mParamIdx, -1, x, y);
			}
		}
		else
		{
			dragParam = kDragLeft;
			dragMinX = mRECT.L;
			dragMaxX = handles[kDragRight].L - handleWidth/2;
		}
	}
	else if (handles[kDragRight].Contains(x, y))
	{
		if (pMod->R)
		{
			SpectralHarp* harp = dynamic_cast<SpectralHarp*>(mPlug);
			if (harp != nullptr)
			{
				harp->BeginMIDILearn(GetAuxParam(kDragRight)->mParamIdx, -1, x, y);
			}
		}
		else
		{
			dragParam = kDragRight;
			dragMinX = handles[kDragLeft].R + handleWidth/2;
			dragMaxX = mRECT.R;
		}
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
			handles[kDragBoth].L = (int)handle.MW();
			SetParamFromHandle(kDragLeft);
			break;

		case kDragRight:
			handles[kDragBoth].R = (int)handle.MW();
			SetParamFromHandle(kDragRight);
			break;

		case kDragBoth:
			handles[kDragLeft].L = handle.L - handleWidth / 2;
			handles[kDragLeft].R = handle.L + handleWidth / 2;
			handles[kDragRight].L = handle.R - handleWidth / 2;
			handles[kDragRight].R = handle.R + handleWidth / 2;
			if ( dX > 0 )
			{
				SetParamFromHandle(kDragRight);
				SetParamFromHandle(kDragLeft);
			}
			else
			{
				SetParamFromHandle(kDragLeft);
				SetParamFromHandle(kDragRight);
			}
			break;
				
		default: break;
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
	pGraphics->FillIRect(&backgroundColor, &backRect);

	pGraphics->DrawLine(&handleColor, backRect.L, backRect.MH(), handles[kDragLeft].L, backRect.MH());
	pGraphics->DrawLine(&handleColor, handles[kDragRight].R, backRect.MH(), backRect.R, backRect.MH());

	pGraphics->FillIRect(&selectedColor, &handles[kDragBoth]);

	DrawHandle(pGraphics, handles[kDragLeft]);
	DrawHandle(pGraphics, handles[kDragRight]);

	return true;
}

void SpectrumSelection::DrawHandle(IGraphics* pGraphics, const IRECT& handle)
{
	int x[4] = { (int)handle.MW(), handle.R, (int)handle.MW(), handle.L };
	int y[4] = { handle.T, (int)handle.MH(), handle.B, (int)handle.MH() };

	//pGraphics->DrawLine(&handleColor, x[0], y[0], x[1], y[1], 0, true);
	//pGraphics->DrawLine(&handleColor, x[1], y[1], x[2], y[2], 0, true);
	//pGraphics->DrawLine(&handleColor, x[2], y[2], x[3], y[3], 0, true);
	//pGraphics->DrawLine(&handleColor, x[3], y[3], x[0], y[0], 0, true);

	pGraphics->FillIConvexPolygon(&handleColor, x, y, 4);
}

void SpectrumSelection::SetParamFromHandle(const int paramIdx)
{
	float mw = handles[paramIdx].MW();
	int hw = handleWidth / 2;
	AuxParam* param = GetAuxParam(paramIdx);
	param->mValue = Map(mw, mRECT.L + hw, mRECT.R - hw, 0, 1);
	mPlug->SetParameterFromGUI(param->mParamIdx, param->mValue);
}

void SpectrumSelection::SetHandleFromParam(const int paramIdx)
{
	int hw = handleWidth / 2;
	int mw = (int)roundf(Map(GetAuxParam(paramIdx)->mValue, 0, 1, mRECT.L+hw, mRECT.R-hw));
	handles[paramIdx].L = mw - hw;
	handles[paramIdx].R = mw + hw;
	switch(paramIdx)
	{
	case kDragLeft:
		handles[kDragBoth].L = mw;
		break;
		
	case kDragRight:
		handles[kDragBoth].R = mw;
		break;
	}
}

void SpectrumSelection::SetAuxParamValueFromPlug(int auxParamIdx, double value)
{
	IControl::SetAuxParamValueFromPlug(auxParamIdx, value);
	SetHandleFromParam(auxParamIdx);
}

bool SpectrumArrows::Draw(IGraphics* pGraphics)
{
	const float a = 2;
	float mid = mRECT.MH();
	pGraphics->DrawLine(&mColor, mRECT.L + a, mid, mRECT.R - a, mid);
	pGraphics->DrawLine(&mColor, mRECT.L + a, mid, mRECT.L + a, mRECT.T);
	pGraphics->DrawLine(&mColor, mRECT.R - a, mid, mRECT.R - a, mRECT.T);
	// left arrow
	pGraphics->DrawLine(&mColor, mRECT.L + a, mRECT.T, mRECT.L, mRECT.T + a);
	pGraphics->DrawLine(&mColor, mRECT.L + a, mRECT.T, mRECT.L + a + a, mRECT.T + a);
	// right arrow
	pGraphics->DrawLine(&mColor, mRECT.R - a, mRECT.T, mRECT.R, mRECT.T + a);
	pGraphics->DrawLine(&mColor, mRECT.R - a, mRECT.T, mRECT.R - a - a, mRECT.T + a);

	return true;
}
