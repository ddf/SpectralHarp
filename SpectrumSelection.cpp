#include "SpectrumSelection.h"
#include "MidiMapper.h"
#include "Params.h"

SpectrumSelection::SpectrumSelection(IRECT rect, int lowParamIdx, int highParamIdx, IColor back, IColor select, IColor handle) 
  : IControl(rect, { lowParamIdx, highParamIdx })
	, backgroundColor(back)
	, selectedColor(select)
	, handleColor(handle)
	, handleWidth(16)
	, dragParam(kDragNone)
{
	const int handT = rect.T;
	const int handB = rect.B;
	handles[kDragLeft] = IRECT(rect.L, handT, rect.L + handleWidth, handB);
	handles[kDragRight] = IRECT(rect.R - handleWidth, handT, rect.R, handB);
	handles[kDragBoth] = IRECT((int)handles[0].L, handT, (int)handles[1].R, handB);
}

SpectrumSelection::~SpectrumSelection()
{
}

// since this control doesn't have it's own param index, we still get right-clicks.
// so we need to reproduce the auto-right-click handling based on which handle is hit.
void SpectrumSelection::OnMouseDown(float x, float y, const IMouseMod& pMod)
{
	if (handles[kDragLeft].Contains(x, y))
	{    
    if (pMod.R)
    {
      for (int i = 0; i < GetUI()->NControls(); ++i)
      {
        if (GetUI()->GetControl(i) == this)
        {
          contextParam = GetParamIdx(kDragLeft);
          GetUI()->ReleaseMouseCapture();
          GetUI()->PopupHostContextMenuForParam(i, contextParam, x, y);
          return;
        }
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
    if (pMod.R)
    {
      for (int i = 0; i < GetUI()->NControls(); ++i)
      {
        if (GetUI()->GetControl(i) == this)
        {
          contextParam = GetParamIdx(kDragRight);
          GetUI()->ReleaseMouseCapture();
          GetUI()->PopupHostContextMenuForParam(i, contextParam, x, y);
          return;
        }
      }
    }
    else
		{
			dragParam = kDragRight;
			dragMinX = handles[kDragLeft].R + handleWidth/2;
			dragMaxX = mRECT.R;
		}
	}
	else if (pMod.L && handles[kDragBoth].Contains(x, y))
	{
		dragParam = kDragBoth;
		dragMinX = mRECT.L + handleWidth / 2;
		dragMaxX = mRECT.R - handleWidth / 2;
	}
}

void SpectrumSelection::OnMouseDrag(float x, float y, float dX, float dY, const IMouseMod& pMod)
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

void SpectrumSelection::OnMouseUp(float x, float y, const IMouseMod& pMod)
{
	dragParam = kDragNone;
}

void SpectrumSelection::CreateContextMenu(IPopupMenu& contextMenu)
{
  MidiMapper* control = dynamic_cast<MidiMapper*>(GetUI()->GetControlWithTag(kMidiMapper));
  if (control != nullptr)
  {
    control->CreateContextMenu(contextMenu, contextParam);
  }
}

void SpectrumSelection::OnContextSelection(int itemSelected)
{
  MidiMapper* control = dynamic_cast<MidiMapper*>(GetUI()->GetControlWithTag(kMidiMapper));
  if (control != nullptr)
  {
    control->OnContextSelection(itemSelected);
  }
}

void SpectrumSelection::Draw(IGraphics& pGraphics)
{
	IRECT backRect = mRECT;
	pGraphics.FillRect(backgroundColor, backRect);

	pGraphics.DrawLine(handleColor, backRect.L, backRect.MH(), handles[kDragLeft].L, backRect.MH());
	pGraphics.DrawLine(handleColor, handles[kDragRight].R, backRect.MH(), backRect.R, backRect.MH());

	pGraphics.FillRect(selectedColor, handles[kDragBoth]);

	DrawHandle(pGraphics, handles[kDragLeft]);
	DrawHandle(pGraphics, handles[kDragRight]);
}

void SpectrumSelection::SetValueFromDelegate(double value, int valIdx)
{
  IControl::SetValueFromDelegate(value, valIdx);
  SetHandleFromParam(GetParamIdx(valIdx));
}

void SpectrumSelection::DrawHandle(IGraphics& pGraphics, const IRECT& handle)
{
	float x[4] = { handle.MW(), handle.R, handle.MW(), handle.L };
	float y[4] = { handle.T, handle.MH(), handle.B, handle.MH() };

	//pGraphics->DrawLine(&handleColor, x[0], y[0], x[1], y[1], 0, true);
	//pGraphics->DrawLine(&handleColor, x[1], y[1], x[2], y[2], 0, true);
	//pGraphics->DrawLine(&handleColor, x[2], y[2], x[3], y[3], 0, true);
	//pGraphics->DrawLine(&handleColor, x[3], y[3], x[0], y[0], 0, true);

	pGraphics.FillConvexPolygon(handleColor, x, y, 4);
}

void SpectrumSelection::SetParamFromHandle(const int paramIdx)
{
	float mw = handles[paramIdx].MW();
	int hw = handleWidth / 2;
  float value = Map(mw, mRECT.L + hw, mRECT.R - hw, 0, 1);
  SetValueFromUserInput(value, paramIdx);
}

void SpectrumSelection::SetHandleFromParam(const int paramIdx)
{
  int id = GetParamIdx(kDragLeft) == paramIdx ? kDragLeft : kDragRight;
	int hw = handleWidth / 2;
	int mw = (int)roundf(Map(GetValue(id), 0, 1, mRECT.L+hw, mRECT.R-hw));
	handles[id].L = mw - hw;
	handles[id].R = mw + hw;
	switch(id)
	{
	case kDragLeft:
		handles[kDragBoth].L = mw;
		break;
		
	case kDragRight:
		handles[kDragBoth].R = mw;
		break;
	}
}

void SpectrumArrows::Draw(IGraphics& pGraphics)
{
	const float a = 2;
	float mid = mRECT.MH();
	pGraphics.DrawLine(mColor, mRECT.L + a, mid, mRECT.R - a, mid);
	pGraphics.DrawLine(mColor, mRECT.L + a, mid, mRECT.L + a, mRECT.T);
	pGraphics.DrawLine(mColor, mRECT.R - a, mid, mRECT.R - a, mRECT.T);
	// left arrow
	pGraphics.DrawLine(mColor, mRECT.L + a, mRECT.T, mRECT.L, mRECT.T + a);
	pGraphics.DrawLine(mColor, mRECT.L + a, mRECT.T, mRECT.L + a + a, mRECT.T + a);
	// right arrow
	pGraphics.DrawLine(mColor, mRECT.R - a, mRECT.T, mRECT.R, mRECT.T + a);
	pGraphics.DrawLine(mColor, mRECT.R - a, mRECT.T, mRECT.R - a - a, mRECT.T + a);
}
