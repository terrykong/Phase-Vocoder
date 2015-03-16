//
//  PSOLA.h
//  
//
//  Created by Terry Kong on 3/5/15.
//
//

#ifndef ____PSOLA__
#define ____PSOLA__

#include <stdio.h>

class PSOLA {
public:
    //PSOLA();
    PSOLA(int bufferLen);
    ~PSOLA();
    void pitchCorrect(int* input, int Fs, float inputPitch, float desiredPitch);
    // Calculates a bartlett window in-place with Q15 coefficients
    void bartlett(int* window, int length);
    
    
private:
    int _bufferLen;
    int* _workingBuffer;
    int* _storageBuffer;
    int* _window;
};


#endif /* defined(____PSOLA__) */
