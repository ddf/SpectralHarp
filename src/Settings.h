//
//  Settings.h
//  melodizer
//
//  Created by Damien Di Fede on 12/12/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#ifndef Settings_h
#define Settings_h

class Settings
{
public:
    
    // global
    static int          BandSpacing;
    static int          BandSpacingMin;
    static int          BandSpacingMax;
    static int          BandOffset;
    static int          BandOffsetMin;
    static int          BandOffsetMax;
	static int			BandLast;
    static float        Decay;
    static float        DecayMin;
    static float        DecayMax;
    static float        BitCrush;
    static float        BitCrushMin;
    static float        BitCrushMax;
    static float        Pitch;
    static float        PitchMin;
    static float        PitchMax;

};

#endif
