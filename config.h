#define PLUG_NAME "SpectralHarp"
#define PLUG_MFR "Damien Quartz"
#define PLUG_VERSION_HEX 0x00020000
#define PLUG_VERSION_STR "2.0.0"
#define PLUG_UNIQUE_ID 'Shrp'
#define PLUG_MFR_ID 'Cpmt'
#define PLUG_URL_STR "https://damikyu.itch.io/spectralharp"
#define PLUG_EMAIL_STR "info@compartmental.net"
#define PLUG_COPYRIGHT_STR "Copyright 2019 Damien Quartz"
#define PLUG_CLASS_NAME SpectralHarp

#define BUNDLE_NAME "SpectralHarp"
#define BUNDLE_MFR "compartmental"
#define BUNDLE_DOMAIN "net"

#define PLUG_CHANNEL_IO "1-1 2-2"

#define PLUG_LATENCY 0
#define PLUG_TYPE 1
#ifdef APP_API
#define PLUG_DOES_MIDI_IN 1
#define PLUG_DOES_MIDI_OUT 1
#else
#define PLUG_DOES_MIDI_IN 1
#define PLUG_DOES_MIDI_OUT 0
#endif
#define PLUG_DOES_MPE 0
#define PLUG_DOES_STATE_CHUNKS 0
#define PLUG_HAS_UI 1
#define PLUG_WIDTH 1024
#define PLUG_HEIGHT 400
#define PLUG_FPS 60
#define PLUG_SHARED_RESOURCES 0

#define AUV2_ENTRY SpectralHarp_Entry
#define AUV2_ENTRY_STR "SpectralHarp_Entry"
#define AUV2_FACTORY SpectralHarp_Factory
#define AUV2_VIEW_CLASS SpectralHarp_View
#define AUV2_VIEW_CLASS_STR "SpectralHarp_View"

#define AAX_TYPE_IDS 'EFN1', 'EFN2'
#define AAX_TYPE_IDS_AUDIOSUITE 'EFA1', 'EFA2'
#define AAX_PLUG_MFR_STR "Acme"
#define AAX_PLUG_NAME_STR "SpectralHarp\nIPEF"
#define AAX_PLUG_CATEGORY_STR "Effect"
#define AAX_DOES_AUDIOSUITE 1

#define VST3_SUBCATEGORY "Instrument|Synth"

#define APP_NUM_CHANNELS 2
#define APP_N_VECTOR_WAIT 0
#define APP_MULT 0.25
#define APP_COPY_AUV3 0
#define APP_RESIZABLE 0
#define APP_SIGNAL_VECTOR_SIZE 64

#define ROBOTTO_FN "Roboto-Regular.ttf"
