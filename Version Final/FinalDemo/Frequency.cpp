/**
 *   @mainpage "@mainpage" TAG WRITES HERE
 *   @author Terry Kong
 *   @date Feb. 28, 2014
 *
 *   \section desc_sec Description
 *   need to finish writing the description
 *
 *  \section contents_sec Table of Contents
 *    Frequency.cpp
 *
 *
 */

/**
 *  @file Frequency.cpp ("@file" TAG WRITES HERE)
 *  @brief ("@brief" TAG WRITES HERE)
 *  @details    More verbose file description.
 */


#include <math.h>
#include "Frequency.h"

#define NUM_PIANO_KEYS          88
#define FIRST_KEY               1
#define NUM_OF_KEYS_IN_OCTAVE   12
#define NUM_OF_KEYS_IN_SCALE    8

/** 
 The first index is "-" so that the index corresponds to the key number
 */
const char *keys[NUM_PIANO_KEYS+1] = {"-","A0","A0s","B0","C1","C1s","D1","D1s","E1","F1","F1s","G1","G1s","A1","A1s","B1","C2","C2s","D2","D2s","E2","F2","F2s","G2","G2s","A2","A2s","B2","C3","C3s","D3","D3s","E3","F3","F3s","G3","G3s","A3","A3s","B3","C4","C4s","D4","D4s","E4","F4","F4s","G4","G4s","A4","A4s","B4","C5","C5s","D5","D5s","E5","F5","F5s","G5","G5s","A5","A5s","B5","C6","C6s","D6","D6s","E6","F6","F6s","G6","G6s","A6","A6s","B6","C7","C7s","D7","D7s","E7","F7","F7s","G7","G7s","A7","A7s","B7","C8"};

/** 
 The first index is -1 so that the index corresponds to the key number
 */
const float keyFreq[NUM_PIANO_KEYS+1] = {-1.0,27.500,29.135,30.868,32.703,34.648,36.708,38.891,41.203,43.654,46.249,48.999,51.913,55.000,58.270,61.735,65.406,69.296,73.416,77.782,82.407,87.307,92.499,97.999,103.826,110.000,116.541,123.471,130.813,138.591,146.832,155.563,164.814,174.614,184.997,195.998,207.652,220.000,233.082,246.942,261.626,277.183,293.665,311.127,329.628,349.228,369.994,391.995,415.305,440.000,466.164,493.883,523.251,554.365,587.330,622.254,659.255,698.456,739.989,783.991,830.609,880.000,932.328,987.767,1046.502,1108.731,1174.659,1244.508,1318.510,1396.913,1479.978,1567.982,1661.219,1760.000,1864.655,1975.533,2093.005,2217.461,2349.318,2489.016,2637.020,2793.826,2959.955,3135.963,3322.438,3520.000,3729.310,3951.066,4186.009};

/**
 Major scale progression
 */
const int majorStep[NUM_OF_KEYS_IN_SCALE - 1] = {2,2,1,2,2,2,1};

/**
 Minor scale progression
 */
const int minorStep[NUM_OF_KEYS_IN_SCALE - 1] = {2,1,2,2,1,2,2};

/** ====================================================
 * @brief       Returns the key number.
 *
 * @details     Returns the key number: if bigger than max or smaller than min, returns NUM_OF_KEYS or 0 respectively.
 *
 * @param       freq          frequency
 *
 * @return      Key number
 *
 * ======================================================
 */
int Frequency::getClosestKeyNum(float freq) {
    if (freq > keyFreq[NUM_PIANO_KEYS]) {
        return NUM_PIANO_KEYS;
    } else if (freq < 0) {
        return FIRST_KEY;
    }
    float minDist;
    for (int i = FIRST_KEY; i <= NUM_PIANO_KEYS; i++) {
        minDist = freq - keyFreq[i];
        // Keep increasing until hit the closest
        if (minDist == 0.0) {
            return i;
        } else if (minDist < 0.0) {
            if (-minDist > (freq - keyFreq[i-1])) {
                return i-1; // the last one is closer
            } else {
                return i; // this current one is closer
            }
        }
    }
    // Something really went wrong
    return -1;
}

/** ====================================================
 * @brief       Returns the closest key number restricted to a scale
 *
 * @details     Returns the key number restricted to a scale: if bigger than max or smaller than min, returns NUM_OF_KEYS or 0 respectively.
 *
 * @param       freq                   frequency
 * @param       keyThatBeginsScale     Pick a key on the piano that begins the scale
 * @param       majorOrMinor           1 = major, -1 = minor, otherwise = major
 *
 * @return      Key number
 *
 * ======================================================
 */
int Frequency::getClosestKeyNumInScale(float freq, int keyThatBeginsScale, int majorOrMinor) {
    if (freq > keyFreq[NUM_PIANO_KEYS]) {
        return NUM_PIANO_KEYS;
    } else if (freq < 0) {
        return FIRST_KEY;
    }
    // Pick either major or minor
    const int* stepProgression;
    if (majorOrMinor == MAJOR_SCALE) {
        stepProgression = majorStep;
    } else if (majorOrMinor == MINOR_SCALE) {
        stepProgression = minorStep;
    } else {
        // assume this is what the user wants
        stepProgression = majorStep;
    }
    // Figure out what the lowest key that keyThatBeginsScale refers to
    keyThatBeginsScale = ((keyThatBeginsScale - 1) % 12) + 1;
    int i = keyThatBeginsScale;
    int indexInScale = 0;
    float minDist;
    while (i <= NUM_PIANO_KEYS) {
        minDist = freq - keyFreq[i];
        // Keep increasing until hit the closest
        if (minDist == 0.0) {
            return i;
        } else if (minDist < 0.0) {
            if (-minDist > (freq - keyFreq[i-1])) {
                return i-1; // the last one is closer
            } else {
                return i; // this current one is closer
            }
        }
        i += stepProgression[indexInScale];
        indexInScale++;
        indexInScale = indexInScale % NUM_OF_KEYS_IN_SCALE;
    }
    // Something really went wrong
    return -1;
}

/** ====================================================
 * @brief       Returns the closest key frequency restricted to a scale
 *
 * @details     Returns the key number restricted to a scale: if bigger than max or smaller than min, returns NUM_OF_KEYS or 0 respectively.
 *
 * @param       freq                   frequency
 * @param       keyThatBeginsScale     Pick a key on the piano that begins the scale
 * @param       majorOrMinor           1 = major, -1 = minor, otherwise = major
 *
 * @return      Key number
 *
 * ======================================================
 */
float Frequency::getClosestKeyFreqInScale(float freq, int keyThatBeginsScale, int majorOrMinor) {
    int keyNum = getClosestKeyNumInScale(freq, keyThatBeginsScale, majorOrMinor);
    return keyFreq[keyNum];
}


/** ====================================================
 * @brief       Returns a pointer to a string holding the name of the key number.
 *
 * @details     Returns a pointer to a string holding the name of the key number. It accepts keys from 1 to NUM_PIANO_KEYS
 *
 * @param       keynum      Key Number
 *
 * @return      Pointer to char* with name of key
 *
 * ======================================================
 */
const char* Frequency::getKeyName(int keynum) {
    return keys[keynum];
}

/** ====================================================
 * @brief       Returns the frequency of a key
 *
 * @details     Returns the frequency of a key: if bigger than max or smaller than min, returns NUM_OF_KEYS or 0 respectively.
 *
 * @param       freq          frequency
 *
 * @return      Key frequency
 *
 * ======================================================
 */
float Frequency::getFreqOfKeyNum(int key) {
    if (key > NUM_PIANO_KEYS) {
        return keyFreq[NUM_PIANO_KEYS];
    } else if (key < FIRST_KEY) {
        return keyFreq[FIRST_KEY];
    }
    return keyFreq[key];
}

