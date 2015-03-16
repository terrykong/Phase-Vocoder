//
//  main.cpp
//  
//
//  Created by Terry Kong on 2/18/15.
//
//

#include <stdio.h>
#include <iostream>
#include "Frequency.h"
#include <string.h>

using namespace std;

int main() {
    Frequency f;
    std::cout << "key name: " << f.getKeyName(49) << std::endl;
    int keyNum = A0s_KEY;
    float keyFreq = f.getFreqOfKeyNum(keyNum);
    std::cout << "closest key: " <<f.getKeyName(keyNum) << " "<<f.getClosestKeyNum(keyFreq)  << std::endl;
    
    int closestKeyNum = f.getClosestKeyNumInScale(keyFreq,C_SCALE,MAJOR_SCALE);
    std::cout << "closest key in c scale: " <<f.getKeyName(closestKeyNum) << " "<< f.getClosestKeyNumInScale(keyFreq,C_SCALE,MAJOR_SCALE) << endl;
    std::cout << "closest key in c scale: " <<f.getKeyName(closestKeyNum) << " "<< f.getClosestKeyFreqInScale(keyFreq,C_SCALE,MAJOR_SCALE) << endl<<endl;
    for(int i = 1; i <= 88; i++) {
        std::cout << "closest key in c scale: " <<f.getKeyName(i) << " "<< f.getClosestKeyFreqInScale(f.getFreqOfKeyNum(i),C_SCALE,MAJOR_SCALE) << endl;
    }
    std::cin.get();
    return 0;
}

