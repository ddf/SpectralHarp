[![License: Zlib](https://img.shields.io/badge/License-Zlib-lightgrey.svg)](https://opensource.org/licenses/Zlib)

# SpectralHarp

SpectralHarp lets you strum the sound spectrum in various ways by giving you a set of strings that activate different frequencies depending on how you configure them. You can choose the portion of the sound spectrum you want to strum as well as how many strings to display from that portion. There are additional knobs for Pitch, Decay, and Crush, which allow you to further modify the sound.

The standalone version also supports controlling parameters with MIDI: simply right-click on a control, select MIDI Learn, and then twiddle a knob on your connected device to map it to that control.

# How to Build

- clone https://github.com/ddf/wdl-ol
- create a folder named Projects in the root of the repo
- clone this repo into the Projects folder
- to build the VST version, follow the instructions here to install the VST SDK: https://github.com/ddf/wdl-ol/tree/master/VST_SDK
- to build the VST3 version, follow the instructions to install the VST 3.6.6 SDK: https://github.com/ddf/wdl-ol/tree/master/VST3_SDK, it should not be necessary to modify any project files
- open SpectralHarp.sln in Visual Studio 2015 or Evaluator.xcodeproj in XCode 9 and build the flavor you are interested in
