//
//  Settings.h
//  SpectralHarp
//
//  Created by Damien Di Fede on 12/12/11.
//  Copyright (c) Compartmental.
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

	static const int    SpectralGenSize = 1024 * 8;
	static const int	BandMin = 12;
	static const int    BandMax = SpectralGenSize / 4;

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
