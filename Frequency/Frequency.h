//
//  Frequency.h
//  
//
//  Created by Terry Kong on 2/18/15.
//
//

#ifndef _Frequency_h
#define _Frequency_h

#define NUM_OF_KEYS_IN_SCALE    8
#define MAJOR_SCALE             1
#define MINOR_SCALE             -1

#define A_SCALE                 1
#define As_SCALE                2
#define B_SCALE                 3
#define C_SCALE                 4
#define Cs_SCALE                5
#define D_SCALE                 6
#define Ds_SCALE                7
#define E_SCALE                 8
#define F_SCALE                 9
#define Fs_SCALE                10
#define G_SCALE                 11
#define Gs_SCALE                12

#define  A0_KEY         1
#define A0s_KEY         2
#define  B0_KEY         3
#define  C1_KEY         4
#define C1s_KEY         5
#define  D1_KEY         6
#define D1s_KEY         7
#define  E1_KEY         8
#define  F1_KEY         9
#define F1s_KEY         10
#define  G1_KEY         11
#define G1s_KEY         12
#define  A1_KEY         13
#define A1s_KEY         14
#define  B1_KEY         15
#define  C2_KEY         16
#define C2s_KEY         17
#define  D2_KEY         18
#define D2s_KEY         19
#define  E2_KEY         20
#define  F2_KEY         21
#define F2s_KEY         22
#define  G2_KEY         23
#define G2s_KEY         24
#define  A2_KEY         25
#define A2s_KEY         26
#define  B2_KEY         27
#define  C3_KEY         28
#define C3s_KEY         29
#define  D3_KEY         30
#define D3s_KEY         31
#define  E3_KEY         32
#define  F3_KEY         33
#define F3s_KEY         34
#define  G3_KEY         35
#define G3s_KEY         36
#define  A3_KEY         37
#define A3s_KEY         38
#define  B3_KEY         39
#define  C4_KEY         40
#define C4s_KEY         41
#define  D4_KEY         42
#define D4s_KEY         43
#define  E4_KEY         44
#define  F4_KEY         45
#define F4s_KEY         46
#define  G4_KEY         47
#define G4s_KEY         48
#define  A4_KEY         49
#define A4s_KEY         50
#define  B4_KEY         51
#define  C5_KEY         52
#define C5s_KEY         53
#define  D5_KEY         54
#define D5s_KEY         55
#define  E5_KEY         56
#define  F5_KEY         57
#define F5s_KEY         58
#define  G5_KEY         59
#define G5s_KEY         60
#define  A5_KEY         61
#define A5s_KEY         62
#define  B5_KEY         63
#define  C6_KEY         64
#define C6s_KEY         65
#define  D6_KEY         66
#define D6s_KEY         67
#define  E6_KEY         68
#define  F6_KEY         69
#define F6s_KEY         70
#define  G6_KEY         71
#define G6s_KEY         72
#define  A6_KEY         73
#define A6s_KEY         74
#define  B6_KEY         75
#define  C7_KEY         76
#define C7s_KEY         77
#define  D7_KEY         78
#define D7s_KEY         79
#define  E7_KEY         80
#define  F7_KEY         81
#define F7s_KEY         82
#define  G7_KEY         83
#define G7s_KEY         84
#define  A7_KEY         85
#define A7s_KEY         86
#define  B7_KEY         87
#define  C8_KEY         88

class Frequency {
public:
    //Frequency();
    //~Frequency();
    int getClosestKeyNum(float freq);
    int getClosestKeyNumInScale(float freq, int keyThatBeginsScale, int majorOrMinor);
    float getClosestKeyFreqInScale(float freq, int keyThatBeginsScale, int majorOrMinor);
    const char* getKeyName(int keynum);
    float getFreqOfKeyNum(int key);
    
};

#endif
