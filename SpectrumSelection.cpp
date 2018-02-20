#include "SpectrumSelection.h"
#include "SpectralHarp.h"
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
#if SA_API
		if (pMod->R)
		{
			SpectralHarp* harp = dynamic_cast<SpectralHarp*>(mPlug);
			if (harp != nullptr)
			{
				harp->BeginMIDILearn(GetAuxParam(kDragLeft)->mParamIdx, -1, x, y);
			}
		}
		else
#endif
		{
			dragParam = kDragLeft;
			dragMinX = mRECT.L;
			dragMaxX = handles[kDragRight].L - handleWidth;
		}
	}
	else if (handles[kDragRight].Contains(x, y))
	{
#if SA_API
		if (pMod->R)
		{
			SpectralHarp* harp = dynamic_cast<SpectralHarp*>(mPlug);
			if (harp != nullptr)
			{
				harp->BeginMIDILearn(GetAuxParam(kDragRight)->mParamIdx, -1, x, y);
			}
		}
		else
#endif
		{
			dragParam = kDragRight;
			dragMinX = handles[kDragLeft].R + handleWidth;
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

void SpectrumSelection::SetHandleFromParam(const int paramIdx)
{
	int hw = handleWidth / 2;
	int mw = (int)roundf(Settings::Map(GetAuxParam(paramIdx)->mValue, 0, 1, mRECT.L+hw, mRECT.R-hw));
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

