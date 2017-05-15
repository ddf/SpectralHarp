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

	static int			BandMin;
	static int			BandMax;
	static int			BandFirst;
	static int			BandLast;
	// 0 - 1 that we use to dynamically figure out how many strings to show based on BandFirst and BandLast
	static float		BandDensity; 
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
