//
//  Settings.cpp
//  melodizer
//
//  Created by Damien Di Fede on 12/12/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#include "Settings.h"


// global
int     Settings::BandSpacing(1);
int     Settings::BandSpacingMin(1);
int     Settings::BandSpacingMax(96);
int     Settings::BandOffset(3);
int     Settings::BandOffsetMin(3);
int     Settings::BandOffsetMax(64);
float   Settings::Decay(1.1f);
float   Settings::DecayMin(0.3f);
float   Settings::DecayMax(1.2f);
float   Settings::BitCrush(44100);
float   Settings::BitCrushMin(Settings::BitCrush);
float   Settings::BitCrushMax(1000);
float   Settings::Pitch( 0.5f );
float   Settings::PitchMin( 0.25f );
float   Settings::PitchMax( 1.0f );

