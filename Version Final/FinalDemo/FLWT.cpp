/**
 *   @mainpage "@mainpage" TAG WRITES HERE
 *   @author Terry Kong
 *   @date Feb. 28, 2014
 *
 *   \section desc_sec Description
 *   need to finish writing the description
 *
 *  \section contents_sec Table of Contents
 *    FLWT.cpp
 *
 *
 */

/**
 *  @file FLWT.cpp ("@file" TAG WRITES HERE)
 *  @brief ("@brief" TAG WRITES HERE)
 *  @details    More verbose file description.
 */

#include "FLWT.h"
#include <math.h>
//#include <iostream> //@debugging
//using namespace std; //@debugging

// ====================================
#define DEFAULT_WIN_LENGTH  1024
#define MAX_LEVELS          6
#define MAX_INT16           32767
#define MIN_INT16           -32768
#define MAX_INT32           2147483647
#define MIN_INT32           -2147483648
// ====================================

/** Sets the number of differences between peaks to consider as the mode
 *
 * @b Example: 
 *
 * @code
 *    peakIndices = [d0,d1,d2,d3,d4,...]
 *    for index = 0:end
 *      for i = 1:MAX_NUM_OF_PEAKS_BETWEEN_MODE
 *                  possibleMode = abs( peakIndices[j+1] - peakIndices[j] )
 * @endcode
 *
 */
#define MAX_NUM_OF_PEAKS_BETWEEN_MODE   3

/** Parameter used when looking for peaks (0 ≤ GLOBAL_MAX_THRESHOLD ≤ 1) */
#define GLOBAL_MAX_THRESHOLD    0.75

/** Any frequency above max is ignored */
#define MAX_FREQUENCY           3000

/** Tolerance when checking if something is an octave off (in Hz) */
#define OCTAVE_TOLERANCE        3

/** Tolerance when checking if the change in frequency is humanly possible */
#define CHANGE_IN_FREQ_TOLERANCE        0.2

#ifndef max
#define max(x, y) ((x) > (y)) ? (x) : (y)
#endif
#ifndef min
#define min(x, y) ((x) < (y)) ? (x) : (y)
#endif

// abs value
int iabs(int x) {
    if (x >= 0) return x;
    return -x;
}

float fabs(float x) {
    if (x >= 0) return x;
    return -x;
}

// load median filter
void FLWT::addToMedianBuffer(float f) {
    if (_medianBufferLastIndex == MEDIAN_BUFFER_LENGTH) {
        _medianBufferLastIndex = 0;
    }
    _medianBuffer5[_medianBufferLastIndex] = f;
    _medianBufferLastIndex++;
}

// Return median
float FLWT::median5() {
    float a = _medianBuffer5[0];
    float b = _medianBuffer5[1];
    float c = _medianBuffer5[2];
    float d = _medianBuffer5[3];
    float e = _medianBuffer5[4];
    // Put the largest value in a
    if (a <= b) { a = a+b; b = a-b; a = a-b; }
    if (a <= c) { a = a+c; c = a-c; a = a-c; }
    if (a <= d) { a = a+d; d = a-d; a = a-d; }
    if (a <= e) { a = a+e; e = a-e; a = a-e; }
    // Put the second largest value in b
    if (b <= c) { b = b+c; c = b-c; b = b-c; }
    if (b <= d) { b = b+d; d = b-d; b = b-d; }
    if (b <= e) { b = b+e; e = b-e; b = b-e; }
    // The 3rd largest is the median
    if (c <= d) { c = c+d; d = c-d; c = c-d; }
    if (c <= e) { c = c+e; e = c-e; c = c-e; }
    return c;
}

/** ==============================================================================
 * @brief       Function initializes fast lifting wavelet transform module for pitch detection.
 *
 * @details     [Verbose description of method
 *              (or function) details.]
 *
 * @todo        [ Finish writing the details]
 *
 * @param       levels       Number of levels for the FLWT algorithm
 * @param       windowLen    Length of the window
 *
 * @see        
 *              http://online.physics.uiuc.edu/courses/phys406/NSF_REU_Reports/2005_reu/Real-Time_Time-Domain_Pitch_Tracking_Using_Wavelets.pdf
 * ================================================================================
 */

FLWT::FLWT(int levels, int windowLen) {
    // Error Handle
    if (windowLen < 4 || windowLen > DEFAULT_WIN_LENGTH) {
        _window = new int[DEFAULT_WIN_LENGTH];
    } else {
        _window = new int[windowLen];
    }
    if (levels < 0 || levels > MAX_LEVELS) {
        _levels = MAX_LEVELS;
    } else {
        _levels = levels;
    }
    _maxCount = new int[levels];
    _minCount = new int[levels];
    _maxIndices = new int[windowLen];
    _minIndices = new int[windowLen];
    _oldFreq = 0.0;
    _oldMode = 0;
    _mode = new int[levels];
    _winLength = windowLen;
    _dLength = 0;
    // This buffer can't overflow up unless windowLen < (#peaks + #valleys)*(#peaks + #valleys + 1)/2
    _differs = new int[windowLen];
    // Median Buffer variables
    _medianBuffer5 = new float[5];
    _medianBufferLastIndex = 0;
}

/** Standard destructor */
FLWT::~FLWT() {
    delete[] _window;
    delete[] _maxCount;
    delete[] _minCount;
    delete[] _maxIndices;
    delete[] _minIndices;
    delete[] _mode;
    delete[] _medianBuffer5;
}

/** ====================================================
 * @brief       Calculates the pitch of a set of data.
 *
 * @details     Uses the FLWT described in Eric Larson and Ross Maddox's paper.
 *              The algorithm was optimized for embedded systems that do not have
 *              dedicated hardware for floating point arithmetic by removing any 
 *              floating point operation that was not deemed necessary to calculate
 *              the pitch. This of course decreases the accuracy of the algorithm,
 *              but at the same time increases the efficiency of the algorithm. If a 
 *              more precise calculation of pitch is desired, check out the URL below 
 *              for a floating point implementation of this algorithm.
 *
 * @param       data        Pointer to array of data
 * @param       datalen     Length of the array
 * @param       fs          Sampling frequency of data
 *
 * @return      Pitch of the window
 *
 * @retval      pitch
 *                      <ul>
 *                         <li> 0.0 : Determines the data is pitchless
 *                         <li> > 0.0 : Found a pitch
 *                      </ul>
 *
 * @see          
 *              http://www.schmittmachine.com/dywapitchtrack.html
 * ======================================================
 */
float FLWT::getPitch(int* data, int datalen, long fs) {
    // Calculate Parameters for this window
    int newWidth = (datalen > _winLength) ? _winLength : datalen;
    long average = 0;
    int globalMax = MIN_INT16;
    int globalMin = MAX_INT16;
    int maxThresh;
    int minThresh;
    for (int i = 0; i < datalen; i++) {
        _window[i] = data[i];
        average += data[i];
        if (data[i] > globalMax) {
            globalMax = data[i];
        }
        if (data[i] < globalMin) {
            globalMin = data[i];
        }
    }
    average /= datalen;
    maxThresh = GLOBAL_MAX_THRESHOLD*(globalMax - average) + average;
    minThresh = GLOBAL_MAX_THRESHOLD*(globalMin - average) + average;
    
    // Perform FLWT Algorithm
    int minDist;
    int climber;
    bool isSearching; // flag for whether or not another peak can be found
    int tooClose; // Make sure the peaks aren't too close
    int test;
    for(int lev = 0; lev < _levels; lev++) {
        // Reinitialize level parameters
        _mode[lev] = 0;
        _maxCount[lev] = 0;
        _minCount[lev] = 0;
        isSearching = true;
        tooClose = 0;
        _dLength = 0;
        
        newWidth = newWidth >> 1;
        minDist = max( floor((fs/MAX_FREQUENCY) >> (lev+1)) ,1);
        // First forward difference of new window (a(i,2) - a(i,1) > 0)
        if ((_window[3]+_window[2]-_window[1]-_window[0]) > 0) {
            climber = 1;
        } else {
            climber = -1;
        }
        
        // Calculate the Approximation component (only) inplace
    
        // First and last element of the approximation is calculated separately
        //  to exploit the fact that maxima and minima can be calculated
        //  while next approximation component is being filled
        _window[0] = (_window[1] + _window[0]) >> 1;
        for (int j = 1; j < newWidth; j++) {
            _window[j] = (_window[2*j+1] + _window[2*j]) >> 1;
            
            // While the window is being filled, find max and mins
            test = _window[j] - _window[j-1]; // first backward difference
            
            if (climber >= 0 && test < 0) { // reached a peak
                if (_window[j-1] >= maxThresh && isSearching && !tooClose) {
                    // value is large enough, haven't found peak yet, and not too close
                    _maxIndices[_maxCount[lev]] = j-1;
                    _maxCount[lev]++;
                    isSearching = false;
                    tooClose = minDist;
                }
                climber = -1;
            } else if(climber <= 0 && test > 0) { // reached valley
                if (_window[j-1] <= minThresh && isSearching && !tooClose) {
                    // value is small enough, haven't found peak yet, and not too close
                    _minIndices[_minCount[lev]] = j-1;
                    _minCount[lev]++;
                    isSearching = false;
                    tooClose = minDist;
                }
                climber = 1;
            
            }
            
            // If we reach zero crossing, we can look for another peak
            if ((_window[j] <= average && _window[j-1] > average) ||
                (_window[j] >= average && _window[j-1] < average)) {
                isSearching = true;
            }
            
            if (tooClose) {
                tooClose--;
            }
        }
        
        // Find the mode distance between peaks
        if (_maxCount[lev] >= 2 && _minCount[lev] >= 2) {
            
            // Find all differences between maxima/minima
            for (int j = 1; j <= MAX_NUM_OF_PEAKS_BETWEEN_MODE; j++) {
                for (int k = 0; k < _maxCount[lev] - j; k++) {
                    _differs[_dLength] = iabs(_maxIndices[k] - _maxIndices[k+j]);
                    _dLength++;
                }
                for (int k = 0; k < _minCount[lev] - j; k++) {
                    _differs[_dLength] = iabs(_minIndices[k] - _minIndices[k+j]);
                    _dLength++;
                }
            }
            
            // Determine the mode
            int numer = 1; // Require at least two agreeing _differs to yield a mode
            int numerJ;
            for (int j = 0; j < _dLength; j++) {
                numerJ = 0;
                
                // Find the number of _differs that are near _differs[j]
                for (int n = 0; n < _dLength; n++) {
                    if (iabs(_differs[j] - _differs[n]) < minDist) {
                        numerJ++;
                    }
                }
                
                // Check to see if there is a better candidate for the mode
                if (numerJ >= numer && numerJ > floor((newWidth/_differs[j])>>2)) {
                    if (numerJ == numer) {
                        if (_oldMode && iabs(_differs[j] - (_oldMode >> (lev+1))) < minDist) {
                            _mode[lev] = _differs[j];
                        } else if (~_oldMode && (_differs[j] > 1.95*_mode[lev] && _differs[j] < 2.05*_mode[lev])) {
                            _mode[lev] = _differs[j];
                        }
                    } else {
                        numer = numerJ;
                        _mode[lev] = _differs[j];
                    }
                } else if (numerJ == numer-1 && _oldMode && iabs(_differs[j] - (_oldMode >> (lev+1))) < minDist) {
                    _mode[lev] = _differs[j];
                }
            }
            
            // Average to get the mode
            if (_mode[lev]) {
                int numerator = 0;
                int denominator = 0;
                for (int m = 0; m < _dLength; m++) {
                    if (iabs(_mode[lev] - _differs[m]) <= minDist) {
                        numerator += _differs[m];
                        denominator++;
                    }
                }
                _mode[lev] = numerator/denominator;
            }
            
            // Check if the mode is shared with the previous level
            if (lev == 0) {
                // Do nothing
            } else if (_mode[lev-1] && _maxCount[lev-1] >= 2 && _minCount[lev-1] >= 2) {
                // If the modes are within a sample of one another, return the calculated frequency
                if (iabs(_mode[lev-1] - 2*_mode[lev]) <= minDist) {
                    _oldMode = _mode[lev-1];
                    _oldFreq = ((float)fs)/((float)_mode[lev-1])/((float)(1<<(lev)));
                    // Add the frequency to the median buffer
                    addToMedianBuffer(_oldFreq);
                    return _oldFreq;
                }
            }
            
        }
    }
    
    // Getting here means the window was pitchless
    
    // Add to this value to the median filter
    addToMedianBuffer(0.0);
    return 0.0;
}

/** ====================================================
 * @brief       Calculates the pitch of a set of data using a median filter.
 *
 * @details     Uses a median filter to calculate the median on the past 5
 *              frequencies including the newest one calculated from the new
 *              set of data. Uses the same algorithm as described in 
 *              FLWT::getpitch. This has a slightly larger overhead to return
 *              the pitch.
 *
 * @param       data        Pointer to array of data
 * @param       datalen     Length of the array
 * @param       fs          Sampling frequency of data
 *
 * @return      Pitch of the window passed through a median filter
 *
 * @retval      pitch
 *
 * ======================================================
 */
float FLWT::getPitchWithMedian5(int* data, int datalen, long fs) {
    this->getPitch(data,datalen,fs);
    return this->median5();
}

/** ====================================================
 * @brief       Calculates the pitch of a set of data.
 *
 * @details     Uses the same algorithm as described in FLWT::getpitch. 
 *              If the current data is pitchless, this returns the last
 *              non-zero result.
 *
 * @param       data        Pointer to array of data
 * @param       datalen     Length of the array
 * @param       fs          Sampling frequency of data
 *
 * @return      Pitch of the window (non-zero)
 *
 * @retval      pitch
 *
 * ======================================================
 */
float FLWT::getPitchLastReliable(int* data, int datalen, long fs) {
    this->getPitch(data,datalen,fs);
    return _oldFreq;
}

/** ====================================================
 * @brief       Calculates the pitch of a set of data.
 *
 * @details     Uses the same algorithm as described in FLWT::getpitch.
 *              If the new pitch is either twice or half the frequency of 
 *              the previous window, the new pitch is considered an error
 *              and the return value is the pitch of the last window.
 *
 * @param       data        Pointer to array of data
 * @param       datalen     Length of the array
 * @param       fs          Sampling frequency of data
 *
 * @return      Pitch of the window that is octave invariant
 *
 * @retval      pitch
 *
 * ======================================================
 */
float FLWT::getPitchOctaveInvariant(int* data, int datalen, long fs) {
    float old = _oldFreq;
    this->getPitch(data,datalen,fs);
    if (_oldFreq >= (2*old - OCTAVE_TOLERANCE) && _oldFreq <= (2*old + OCTAVE_TOLERANCE)) {
        _oldFreq = old;
    } else if (old >= (2*_oldFreq - OCTAVE_TOLERANCE) && old <= (2*_oldFreq + OCTAVE_TOLERANCE)) {
        _oldFreq = old;
    }
    return _oldFreq;
}

/** ====================================================
 * @brief       Calculates the pitch of a set of data.
 *
 * @details     Uses the same algorithm as described in FLWT::getpitch.
 *              It uses a robust algorithm that is a combination of all the
 *              methods included in this class.
 *
 * @param       data        Pointer to array of data
 * @param       datalen     Length of the array
 * @param       fs          Sampling frequency of data
 *
 * @return      Pitch of the window
 *
 * @retval      pitch
 *
 * ======================================================
 */
float FLWT::getPitchRobust(int* data, int datalen, long fs) {
    // Calculate Parameters for this window
    float currentFreq;
    int newWidth = (datalen > _winLength) ? _winLength : datalen;
    long average = 0;
    int globalMax = MIN_INT16;
    int globalMin = MAX_INT16;
    int maxThresh;
    int minThresh;
    for (int i = 0; i < datalen; i++) {
        _window[i] = data[i];
        average += data[i];
        if (data[i] > globalMax) {
            globalMax = data[i];
        }
        if (data[i] < globalMin) {
            globalMin = data[i];
        }
    }
    average /= datalen;
    maxThresh = GLOBAL_MAX_THRESHOLD*(globalMax - average) + average;
    minThresh = GLOBAL_MAX_THRESHOLD*(globalMin - average) + average;
    
    // Perform FLWT Algorithm
    int minDist;
    int climber;
    bool isSearching; // flag for whether or not another peak can be found
    int tooClose; // Make sure the peaks aren't too close
    int test;
    for(int lev = 0; lev < _levels; lev++) {
        // Reinitialize level parameters
        _mode[lev] = 0;
        _maxCount[lev] = 0;
        _minCount[lev] = 0;
        isSearching = true;
        tooClose = 0;
        _dLength = 0;
        
        newWidth = newWidth >> 1;
        minDist = max( floor((fs/MAX_FREQUENCY) >> (lev+1)) ,1);
        // First forward difference of new window (a(i,2) - a(i,1) > 0)
        if ((_window[3]+_window[2]-_window[1]-_window[0]) > 0) {
            climber = 1;
        } else {
            climber = -1;
        }
        
        // Calculate the Approximation component (only) inplace
        
        // First and last element of the approximation is calculated separately
        //  to exploit the fact that maxima and minima can be calculated
        //  while next approximation component is being filled
        _window[0] = (_window[1] + _window[0]) >> 1;
        for (int j = 1; j < newWidth; j++) {
            _window[j] = (_window[2*j+1] + _window[2*j]) >> 1;
            
            // While the window is being filled, find max and mins
            test = _window[j] - _window[j-1]; // first backward difference
            
            if (climber >= 0 && test < 0) { // reached a peak
                if (_window[j-1] >= maxThresh && isSearching && !tooClose) {
                    // value is large enough, haven't found peak yet, and not too close
                    _maxIndices[_maxCount[lev]] = j-1;
                    _maxCount[lev]++;
                    isSearching = false;
                    tooClose = minDist;
                }
                climber = -1;
            } else if(climber <= 0 && test > 0) { // reached valley
                if (_window[j-1] <= minThresh && isSearching && !tooClose) {
                    // value is small enough, haven't found peak yet, and not too close
                    _minIndices[_minCount[lev]] = j-1;
                    _minCount[lev]++;
                    isSearching = false;
                    tooClose = minDist;
                }
                climber = 1;
                
            }
            
            // If we reach zero crossing, we can look for another peak
            if ((_window[j] <= average && _window[j-1] > average) ||
                (_window[j] >= average && _window[j-1] < average)) {
                isSearching = true;
            }
            
            if (tooClose) {
                tooClose--;
            }
        }
        
        // Find the mode distance between peaks
        if (_maxCount[lev] >= 2 && _minCount[lev] >= 2) {
            
            // Find all differences between maxima/minima
            for (int j = 1; j <= MAX_NUM_OF_PEAKS_BETWEEN_MODE; j++) {
                for (int k = 0; k < _maxCount[lev] - j; k++) {
                    _differs[_dLength] = iabs(_maxIndices[k] - _maxIndices[k+j]);
                    _dLength++;
                }
                for (int k = 0; k < _minCount[lev] - j; k++) {
                    _differs[_dLength] = iabs(_minIndices[k] - _minIndices[k+j]);
                    _dLength++;
                }
            }
            
            // Determine the mode
            int numer = 1; // Require at least two agreeing _differs to yield a mode
            int numerJ;
            for (int j = 0; j < _dLength; j++) {
                numerJ = 0;
                
                // Find the number of _differs that are near _differs[j]
                for (int n = 0; n < _dLength; n++) {
                    if (iabs(_differs[j] - _differs[n]) < minDist) {
                        numerJ++;
                    }
                }
                
                // Check to see if there is a better candidate for the mode
                if (numerJ >= numer && numerJ > floor((newWidth/_differs[j])>>2)) {
                    if (numerJ == numer) {
                        if (_oldMode && iabs(_differs[j] - (_oldMode >> (lev+1))) < minDist) {
                            _mode[lev] = _differs[j];
                        } else if (~_oldMode && (_differs[j] > 1.95*_mode[lev] && _differs[j] < 2.05*_mode[lev])) {
                            _mode[lev] = _differs[j];
                        }
                    } else {
                        numer = numerJ;
                        _mode[lev] = _differs[j];
                    }
                } else if (numerJ == numer-1 && _oldMode && iabs(_differs[j] - (_oldMode >> (lev+1))) < minDist) {
                    _mode[lev] = _differs[j];
                }
            }
            
            // Average to get the mode
            if (_mode[lev]) {
                int numerator = 0;
                int denominator = 0;
                for (int m = 0; m < _dLength; m++) {
                    if (iabs(_mode[lev] - _differs[m]) <= minDist) {
                        numerator += _differs[m];
                        denominator++;
                    }
                }
                _mode[lev] = numerator/denominator;
            }
            
            // Check if the mode is shared with the previous level
            if (lev == 0) {
                // Do nothing
            } else if (_mode[lev-1] && _maxCount[lev-1] >= 2 && _minCount[lev-1] >= 2) {
                // If the modes are within a sample of one another, return the calculated frequency
                if (iabs(_mode[lev-1] - 2*_mode[lev]) <= minDist) {
                    _oldMode = _mode[lev-1];
                    currentFreq = ((float)fs)/((float)_mode[lev-1])/((float)(1<<(lev)));
                    // Add the frequency to the median buffer
                    //addToMedianBuffer(_oldFreq);
                    //return _oldFreq;
                    goto wrapUp;
                }
            }
            
        }
    }
    
    // Getting here means the window was pitchless
    currentFreq = 0.0;
    // Add to this value to the median filter
    //addToMedianBuffer(0.0);
    //return 0.0;
    
wrapUp:
    // Check whether the change in frequency is humanly possible
    /*if (fabs(currentFreq - _oldFreq) < _oldFreq*CHANGE_IN_FREQ_TOLERANCE) {
        // If the change wasn't significant then we should use the median filter
        if (currentFreq) {
            addToMedianBuffer(currentFreq);
            _oldFreq = this->median5();
            return _oldFreq;
        } else { // Current freq == 0
            _oldFreq = currentFreq;
            addToMedianBuffer(currentFreq);
            return currentFreq;
        }
    } else {
        // New frequency could be an octave change, but the median filter should
        //  take care of it
        addToMedianBuffer(currentFreq);
        _oldFreq = currentFreq;
        return _oldFreq;
    }*/
    float currentMedian;
    if (currentFreq == 0.0) {
        currentMedian = this->median5();
        // check if median is zero
        if (currentMedian == 0.0) {
            addToMedianBuffer(_oldFreq);
            return _oldFreq;
        } else {
            addToMedianBuffer(currentMedian);
            return currentMedian;
        }
    } else {
        addToMedianBuffer(currentFreq);
        _oldFreq = this->median5();
        return _oldFreq;
    }
}

