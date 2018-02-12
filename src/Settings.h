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

	static float Map(float value, float istart, float istop, float ostart, float ostop)
	{
		return ostart + (ostop - ostart) * ((value - istart) / (istop - istart));
	}
    
    // global
    static int          BandSpacing;
    static int          BandSpacingMin;
    static int          BandSpacingMax;
    static int          BandOffset;
    static int          BandOffsetMin;
    static int          BandOffsetMax;

	static const int    SpectralAmpMax  = 256;
	static const int    SpectralGenSize = 1024 * 8;
	static const int	BandMin = 12;
	static const int    BandMax = (int)(SpectralGenSize * 0.3);

	static const int	BandDensityMin = 16;
	static const int	BandDesityMax = 256;

	static int			BandFirst;
	static int			BandLast;
	// how many strings to show
	static int   		BandDensity;
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
