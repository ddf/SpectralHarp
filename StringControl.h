#pragma once
#include "IControl.h"
#include "Params.h"

using namespace iplug;
using namespace igraphics;

class SpectralGen;

class StringControl : public IControl
{
public:
  // data packet that the DSP sends with spectrum information.
  // transmission of Data follows the pattern found in IVScopeControl
  struct Data
  {
    // actual size of the spectrum
    int specSize = kSpectralSizeMax/2;
    // sample rate of the data in the spectrum (needed for freqToIndex)
    int sampleRate = 44100;
    // the bandwidth of each band in the spectrum
    float bandWidth = 1;
    // phase and magnitude for each band in the spectrum
    float phase[kSpectralSizeMax/2];
    float mag[kSpectralSizeMax/2];

    // convert a frequency in Hz to a index into the above arrays
    int freqToIndex(const float freq)
    {
      // special case: freq is lower than the bandwidth of spectrum[0]
      if (freq < bandWidth / 2) return 0;
      // special case: freq is within the bandwidth of spectrum[spectrum.length - 1]
      if (freq > sampleRate / 2 - bandWidth / 2) return specSize - 1;
      // all other cases
      float fraction = freq / sampleRate;
      // roundf is not available in windows, so we do this
      int i = (int)floorf((float)specSize * 2 * fraction + 0.5f);
      return i;
    }
  };

  class Capture
  {
  public:
    Capture() : mTransmit(false) {}

    // called from main DSP thread in ProcessBlock
    void ProcessSpectrum(const SpectralGen& spectrum);

    // called from main DSP thread in OnIdle
    void TransmitData(IEditorDelegate& dlg);

  private:
    Data mData;
    bool mTransmit;
  };

	StringControl(IRECT pR, int handleRadius);

	void Draw(IGraphics&) override;

	void OnMouseDown(float x, float y, const IMouseMod& pMod) override;

	void OnMouseUp(float x, float y, const IMouseMod& pMod) override;

	void OnMouseDrag(float x, float y, float dX, float dY, const IMouseMod& pMod) override;

  void CreateContextMenu(IPopupMenu& contextMenu) override;
  void OnContextSelection(int itemSelected) override;

  void OnMsgFromDelegate(int messageTag, int dataSize, const void* pData) override;

private:

	void SnapToMouse(float x, float y);

  Data mSpectrum;
	int mHandleRadius;
	float mMouseX;
	float mMouseY;
	float stringAnimation;
};

