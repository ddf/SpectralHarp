#include "MidiMapper.h"

#include "Params.h"
#include "IControl.h"
#include "IGraphicsPopupMenu.h"


MidiMapper::MidiMapper()
: IControl(IRECT())
, mItemIndex1(-1)
, mItemIndex2(-1)
, mLearnParamIdx(-1)
{
  SetTag(kMidiMapper);
}

MidiMapper::~MidiMapper()
{
}

void MidiMapper::CreateContextMenu(IPopupMenu& contextMenu, const int paramIdx1, const int paramIdx2)
{
  mParamIdx1 = paramIdx1;
  mParamIdx2 = paramIdx2;
  mItemIndex1 = -1;
  mItemIndex2 = -1;
  WDL_String str;

  if (paramIdx1 != -1)
  {
    bool isMapped = controlChangeForParam[paramIdx1] != MidiMapping::kNone;
    int flags = isMapped ? IPopupMenu::Item::kChecked : IPopupMenu::Item::kNoFlags;
    if (isMapped)
    {
      str.SetFormatted(64, "MIDI Learn: %s (CC %d)", GetDelegate()->GetParam(paramIdx1)->GetNameForHost(), (int)controlChangeForParam[paramIdx1]);
    }
    else
    {
      str.SetFormatted(64, "MIDI Learn: %s", GetDelegate()->GetParam(paramIdx1)->GetNameForHost());
    }
    mItemIndex1 = contextMenu.NItems();
    contextMenu.AddItem(str.Get(), -1, flags);
  }

  if (paramIdx2 != -1)
  {
    bool isMapped = controlChangeForParam[paramIdx2] != MidiMapping::kNone;
    int flags = isMapped ? IPopupMenu::Item::kChecked : IPopupMenu::Item::kNoFlags;
    if (isMapped)
    {
      str.SetFormatted(64, "MIDI Learn: %s (CC %d)", GetDelegate()->GetParam(paramIdx2)->GetNameForHost(), (int)controlChangeForParam[paramIdx2]);
    }
    else
    {
      str.SetFormatted(64, "MIDI Learn: %s", GetDelegate()->GetParam(paramIdx2)->GetNameForHost());
    }
    mItemIndex2 = contextMenu.NItems();
    contextMenu.AddItem(str.Get(), -1, flags);

    contextMenu.SetMultiCheck(paramIdx1 != -1);
  }
}

void MidiMapper::OnContextSelection(int itemIndex)
{
  if (itemIndex == mItemIndex1 && mParamIdx1 != -1)
  {
    if (controlChangeForParam[mParamIdx1] == MidiMapping::kNone)
    {
      mLearnParamIdx = mParamIdx1;
      SetWantsMidi(true);
    }
    else
    {
      controlChangeForParam[mParamIdx1] = MidiMapping::kNone;
      // update the processor
      MidiMapping map(mParamIdx1);
      GetDelegate()->SendArbitraryMsgFromUI(kSetMidiMapping, GetTag(), sizeof(MidiMapping), &map);
    }
  }
  else if (itemIndex == mItemIndex2 && mParamIdx2 != -1)
  {
    if (controlChangeForParam[mParamIdx2] == MidiMapping::kNone)
    {
      mLearnParamIdx = mParamIdx2;
      SetWantsMidi(true);
    }
    else
    {
      controlChangeForParam[mParamIdx2] = MidiMapping::kNone;
      // update the processor
      MidiMapping map(mParamIdx2);
      GetDelegate()->SendArbitraryMsgFromUI(kSetMidiMapping, GetTag(), sizeof(MidiMapping), &map);
    }
  }
}

void MidiMapper::OnMidi(const IMidiMsg& msg)
{
  if (mLearnParamIdx != -1 && msg.StatusMsg() == IMidiMsg::kControlChange)
  {
    controlChangeForParam[mLearnParamIdx] = (MidiMapping::CC)msg.ControlChangeIdx();

    // update the processor
    MidiMapping map(mLearnParamIdx, controlChangeForParam[mLearnParamIdx]);
    GetDelegate()->SendArbitraryMsgFromUI(kSetMidiMapping, mLearnParamIdx, sizeof(MidiMapping), &map);

    mLearnParamIdx = -1;
    SetWantsMidi(false);
  }
}

void MidiMapper::OnMsgFromDelegate(int messageTag, int dataSize, const void* pData)
{
  if (messageTag == kSetMidiMapping && dataSize == sizeof(MidiMapping))
  {
    const MidiMapping* map = (MidiMapping*)pData;
    controlChangeForParam[map->param] = map->midiCC;
  }
}

void MidiMapper::Draw(IGraphics& g)
{
  // nothing to draw
}