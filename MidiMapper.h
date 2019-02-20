#pragma once

#include "IControl.h"
#include "Params.h"

// class that implements Midi Learn functionality in a generic way for use by our IControl subclasses.
// they will delegate the callback work by retrieving the single instance of this control using GetControlWithTag.
class MidiMapper : public IControl
{
public:
	MidiMapper();
	~MidiMapper();

  void CreateContextMenu(IPopupMenu& contextMenu, const int paramIdx1, const int paramIdx2 = -1);
  void OnContextSelection(int index) override;
  void OnMidi(const IMidiMsg& msg) override;
  // allows the delegate to initialize / change midi mappings
  void OnMsgFromDelegate(int messageTag, int dataSize, const void* pData) override;

  // required, but we don't draw anything
  void Draw(IGraphics& g) override;

private:
  // params in the menu
  int mParamIdx1, mParamIdx2;
  // popup menu indices
  int mItemIndex1, mItemIndex2;

  // if not -1 when we receive a control change midi message
  // we use this to determine which param should be linked to the control change
  int  mLearnParamIdx;
  // for each param, which midi control change should set its value
  MidiMapping::CC controlChangeForParam[kNumParams];
};

