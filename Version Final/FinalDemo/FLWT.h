//
//  FLWT.h
//  
//
//  Created by Terry Kong on 2/21/15.
//
//

#ifndef ____FLWT__
#define ____FLWT__

#include <stdio.h>

#define DEFAULT_WIN_LENGTH  1024
#define MEDIAN_BUFFER_LENGTH 5

/**
 * @brief      Short class description.
 *
 * @details    Verbose description of class details.
 *
 * Note the following example code:
 * @code
 *    Window win = new Window(parent);
 *    win.show();
 * @endcode
 */

class FLWT {
public:
    //FLWT();
    // windowLen MUST be divisible by 2^(levels-1)
    FLWT(int levels, int windowLen = DEFAULT_WIN_LENGTH);
    ~FLWT();
    // datalen MUST be divisible by 2^(levels-1)
    // Returns 0.0 if it deduces the segment is pitchless
    float getPitch(int* data, int datalen, long fs);
    float getPitchWithMedian5(int* data, int datalen, long fs);
    float getPitchLastReliable(int* data, int datalen, long fs);
    float getPitchOctaveInvariant(int* data, int datalen, long fs);
    float getPitchRobust(int* data, int datalen, long fs);
    
private:
    void addToMedianBuffer(float f);
    float median5();
    int *_window;
    int _levels;
    int *_maxCount;
    int *_minCount;
    int *_maxIndices;
    int *_minIndices;
    float _oldFreq;
    int _oldMode;
    int *_mode;
    int _winLength;
    int _dLength;
    int *_differs;
    float *_medianBuffer5;
    int _medianBufferLastIndex;
};

#endif /* defined(____FLWT__) */
