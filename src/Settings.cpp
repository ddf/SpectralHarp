//
//  Settings.cpp
//  melodizer
//
//  Created by Damien Di Fede on 12/12/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#include "Settings.h"
#include "SpectralGen.h"

// global
int     Settings::BandSpacing(50);
int     Settings::BandSpacingMin(3);
int     Settings::BandSpacingMax(96);
int     Settings::BandOffset(10);
int     Settings::BandOffsetMin(Settings::BandOffset);
int     Settings::BandOffsetMax(64);
int		Settings::BandMin(1);
int     Settings::BandMax(kSpectralGenSize / 4);
int		Settings::BandFirst(Settings::BandMin);
int		Settings::BandLast(Settings::BandMax);
float   Settings::BandDensity(0.95f);
float   Settings::Decay(0.2f);
float   Settings::DecayMin(0.8f);
float   Settings::DecayMax(0.0f);
float   Settings::BitCrush(44100);
float   Settings::BitCrushMin(Settings::BitCrush);
float   Settings::BitCrushMax(1000);
float   Settings::Pitch( 1.0f );
float   Settings::PitchMin( 0.25f );
float   Settings::PitchMax( 1.0f );

