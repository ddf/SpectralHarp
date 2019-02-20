#include "SpectralHarp.h"
#include "StringControl.h"
#include "MidiMapper.h"
#include "SpectralGen.h"
#include "Params.h"

const float kPadding = 16;

StringControl::StringControl(const SpectralGen& rSpectrum, IRECT pR, int handleRadius) 
	: IControl(pR, kPluckX) // note: we only set kPluckX as our param for the right-click context menu functionality
	, spectrum(rSpectrum)
	, mHandleRadius(handleRadius)
	, mMouseX(-1)
	, mMouseY(-1)
	, stringAnimation(0)
{
}

void StringControl::Draw(IGraphics& pGraphics)
{
  // #TODO send these param changes a messages so we don't have to GetDelegate?
	//const int numBands = (Settings::BandLast - Settings::BandFirst) * Settings::BandDensity;
	const float numBands = (float)GetDelegate()->GetParam(kBandDensity)->Int();
	const float bandFirst = (float)GetDelegate()->GetParam(kBandFirst)->Int();
	const float bandLast = (float)GetDelegate()->GetParam(kBandLast)->Int();

	SpectralHarp* harp = static_cast<SpectralHarp*>(GetDelegate());
  if ( numBands > 0 && harp != nullptr)
  {    
    for (int b = 0; b <= numBands; ++b)
    {
      const float freq = harp->FrequencyOfString(b);
      const float x = Map((float)b, 0, numBands, mRECT.L + kPadding, mRECT.R - kPadding);
      const float p = spectrum.getBandPhase(freq) + stringAnimation;
      const float m = spectrum.getBandMagnitude(freq);

      const int g = (int)(255.f * Map(m, 0, kSpectralAmpMax, 0.4f, 1.f));
      const IColor color(255, g, g, g);

      const float w = Map(m, 0, kSpectralAmpMax, 0, 6);
      const float segments = w > 0 ? 32 : 1;
      const float segLength = mRECT.H() / segments;
      float py0 = 0;
      float px0 = x + w * sinf(p);
      pGraphics.PathMoveTo(px0, py0);
      for (int i = 1; i < segments + 1; ++i)
      {
        const float py1 = i * segLength;
        const float s1 = py1 / mRECT.H() * (float)M_PI * 8 + p;
        const float px1 = x + w * sinf(s1);
        pGraphics.PathLineTo(px1, py1);
        px0 = px1;
        py0 = py1;
      }
      pGraphics.PathStroke(color, 1);
    }
  }
    
	const float dt = 1.0f / 60.f;

	stringAnimation += dt * TWO_PI * 2;
	if (stringAnimation > TWO_PI)
	{
		stringAnimation -= TWO_PI;
	}

	if (mMouseX != -1 || mMouseY != -1)
	{
		SnapToMouse(mMouseX, mMouseY);
	}

	SetDirty(false);
}

void StringControl::OnMouseDown(float x, float y, const IMouseMod& pMod)
{
  if ( pMod.L )
	{
		mMouseX = x;
		mMouseY = y;
		SnapToMouse(x, y);
	}
}

void StringControl::OnMouseUp(float x, float y, const IMouseMod& pMod)
{
	mMouseX = -1;
	mMouseY = -1;
}

void StringControl::OnMouseDrag(float x, float y, float dX, float dY, const IMouseMod& pMod)
{
	if (pMod.L)
	{
		mMouseX = x;
		mMouseY = y;
		SnapToMouse(x, y);
	}
}

void StringControl::CreateContextMenu(IPopupMenu& contextMenu)
{
  MidiMapper* control = dynamic_cast<MidiMapper*>(GetUI()->GetControlWithTag(kMidiMapper));
  if (control != nullptr)
  {
    control->CreateContextMenu(contextMenu, kPluckX, kPluckY);
  }
}

void StringControl::OnContextSelection(int itemSelected)
{
  MidiMapper* control = dynamic_cast<MidiMapper*>(GetUI()->GetControlWithTag(kMidiMapper));
  if (control != nullptr)
  {
    control->OnContextSelection(itemSelected);
  }
}

void StringControl::SnapToMouse(float x, float y)
{
	const float pluckX = Clip(Map((float)x, mRECT.L + kPadding, mRECT.R - kPadding, 0, 1), 0.f, 1.f);
	const float pluckY = Clip(Map((float)y, mRECT.T, mRECT.B, 1, 0), 0.f, 1.f);

  PluckCoords coords(pluckX, pluckY);
  GetDelegate()->SendArbitraryMsgFromUI(kPluckSpectrum, GetTag(), sizeof(PluckCoords), &coords);
}
