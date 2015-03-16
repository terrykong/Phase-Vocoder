/** \mainpage Spectrum Analysis template
 * 
 * Demonstrates DMA ISR implementation.
 * 
 * Reads data from codec audio IN, process the data and sends the
 * output to the codec OUT which can be listened on headphone.
 */

#include "serial_array.h"
#include "Audio_new.h"
#include <OLED.h>
//#include <dsplib.h>
#include "FLWT.h"
#include "PSOLA.h"
#include "Frequency.h"

//===============================================================
// Helper Functions/Function Definitions ========================
//===============================================================
#ifndef max
#define max(x, y) ((x) > (y)) ? (x) : (y)
#endif
#ifndef min
#define min(x, y) ((x) < (y)) ? (x) : (y)
#endif
//===============================================================
// Globals ======================================================
//===============================================================
const int  BufferLength = 512;
//===============================================================
// MATLAB Variables =============================================
//===============================================================
int USING_MATLAB_OR_SERIAL_MONITOR = true; // true = MATLAB, false = SERIAL MONITOR
const int matlabBufLen = 512;
union {
  float freqF[matlabBufLen];
  long freqL[matlabBufLen];
  char freqB[matlabBufLen*4];
} un;
volatile int freqIndex = 0;
//===============================================================
// Declare FLWT object ==========================================
//===============================================================
// Set codec sampling rate:
//   SAMPLING_RATE_8_KHZ
//   SAMPLING_RATE_11_KHZ
//   SAMPLING_RATE_12_KHZ
//   SAMPLING_RATE_16_KHZ
//   SAMPLING_RATE_22_KHZ
//   SAMPLING_RATE_24_KHZ
//   SAMPLING_RATE_32_KHZ
//   SAMPLING_RATE_44_KHZ
//   SAMPLING_RATE_48_KHZ (default)
long fs_default = SAMPLING_RATE_32_KHZ;
int levels = 6;
int windowLen = BufferLength;
FLWT flwt(levels,windowLen);
//===============================================================
// Declare PSOLA object =========================================
//===============================================================
PSOLA psola(BufferLength);
//===============================================================
// DIP switch flags and parameters ==============================
//===============================================================
enum STATUS {NORMAL_MODE,PHASE_VOCODER,OCTAVE_UP,OCTAVE_DOWN,PITCH_DETECT};
volatile int CURRENT_STATUS = NORMAL_MODE;

const long NORMAL_MODE_SAMPLING_RATE =   SAMPLING_RATE_48_KHZ;
const long PHASE_VOCODER_SAMPLING_RATE = SAMPLING_RATE_32_KHZ; // need to figure this out @@
const int  PLAY_PITCHLESS = true; // play even if no pitch is found in the signal 
const float  HEURISTIC_FACTOR = 1; // A factor that tries to help make the pitch correction better
const long OCTAVE_DOWN_SAMPLING_RATE =   SAMPLING_RATE_8_KHZ;
const long OCTAVE_UP_SAMPLING_RATE =     SAMPLING_RATE_16_KHZ;
const float OCTAVE_DOWN_APPARENT_FREQ = 200;
const float OCTAVE_UP_APPARENT_FREQ = 1500;
const long PITCH_DETECT_SAMPLING_RATE =  SAMPLING_RATE_32_KHZ;

const float TimeInSecUntilMatlabUpdate = 0.25;
const int MaxNumOfBufferReads =(int) min(matlabBufLen,TimeInSecUntilMatlabUpdate/((float)BufferLength/PITCH_DETECT_SAMPLING_RATE));
volatile int numOfBufferReads = 0;
//===============================================================
// Piano Key Module==============================================
//===============================================================
Frequency f;
volatile float freq;
volatile int closestKeyNum;
volatile float closestFreq;
const int activeScale = C_SCALE;
const int activeMajorOrMinor = MAJOR_SCALE;
//===============================================================
//===============================================================

// Baud rate
const long baudRate = 115200;

// Max serial communications buffer length
const int maxDataLength = 2048;

#pragma DATA_ALIGN(32)
int output[maxDataLength] = {0};

// Create instance of the serial command class
SerialCmd cmd(maxDataLength);

// Input/output buffer pointers
#pragma DATA_ALIGN(32)
int InputLeft[BufferLength];
#pragma DATA_ALIGN(32)
int InputRight[BufferLength];
#pragma DATA_ALIGN(32)
int OutputLeft[BufferLength];
#pragma DATA_ALIGN(32)
int OutputRight[BufferLength];

// New declarations
int window[BufferLength];

/** \brief Setup function
 * 
 * Allocate memory for input/output arrays.
 * 
 * Initialize OLED display and Audio module.
 * 
 */
void setup()
{ 
  // Window Initialization (rectangle window with unit height)
  for(int i = 0; i < BufferLength; i++) {
    window[i] = 1;
  }

  int status;
  int index;

  pinMode(LED0, OUTPUT);
  digitalWrite(LED0, HIGH);

  //Initialize OLED module for status display	
  disp.oledInit();
  disp.clear();
  disp.flip();    
  disp.setline(1);

  // Audio library is configured for non-loopback mode
  status = AudioC.Audio(TRUE, BufferLength);

  // Set codec sampling rate:
  //   SAMPLING_RATE_8_KHZ
  //   SAMPLING_RATE_11_KHZ
  //   SAMPLING_RATE_12_KHZ
  //   SAMPLING_RATE_16_KHZ
  //   SAMPLING_RATE_22_KHZ
  //   SAMPLING_RATE_24_KHZ
  //   SAMPLING_RATE_32_KHZ
  //   SAMPLING_RATE_44_KHZ
  //   SAMPLING_RATE_48_KHZ (default)
  AudioC.setupCodec(fs_default);

  if (status == 0)
  {
    disp.print("Process ON");
    AudioC.attachIntr(dmaIsr);
  }

  // Set audio output volume (0 for line-level input, 100 for a passive microphone)
  //AudioC.setOutputVolume(25);
  AudioC.setInputGain(25,25);

  if (USING_MATLAB_OR_SERIAL_MONITOR) {
    // Connect to Matlab
    serial_connect(baudRate);

    disp.clear();
    disp.setline(0);
    disp.print("recording");
    freqIndex = 0;
  } 
  else {
    Serial.begin(baudRate);
    disp.clear();
    disp.setline(0);
    disp.print("recording");
  }
}

/** \brief Main application loop
 *
 */
void loop() {
    int command;
    cmd.recv();        
    command = cmd.getCmd();
    parse(command);   
}

/** \brief Main processing function
 * 
 * \param inputLeft is a pointer to the input data array for the left channel
 * \param inputRight is a pointer to the input data array for the right channel
 * \param outputLeft is a pointer to the output data array for the left channel
 * \param outputRight is a pointer to the output data array for the right channel
 * 
 */
void processData(const int *inputLeft, const int *inputRight, int *outputLeft, int *outputRight)
{
  for(int n = 0; n < BufferLength; n++)
  {
    outputLeft[n]  = inputLeft[n]*window[n];
    outputRight[n] = inputRight[n]*window[n];
  }
  
  switch(CURRENT_STATUS) {
   case NORMAL_MODE :
       // don't need to do anything
       break;
   case PHASE_VOCODER :
       freq = flwt.getPitchWithMedian5(outputLeft,BufferLength,PHASE_VOCODER_SAMPLING_RATE);
       if (freq) {
         closestFreq = f.getClosestKeyFreqInScale(freq,activeScale,activeMajorOrMinor);
         psola.pitchCorrect(outputLeft,PHASE_VOCODER_SAMPLING_RATE,freq,closestFreq);
         psola.pitchCorrect(outputRight,PHASE_VOCODER_SAMPLING_RATE,freq,closestFreq);
       } 
       break;
   case OCTAVE_UP :
       // Double the frequency
       freq = OCTAVE_UP_APPARENT_FREQ;
       psola.pitchCorrect(outputLeft,OCTAVE_UP_SAMPLING_RATE,freq,freq*2);
       psola.pitchCorrect(outputRight,OCTAVE_UP_SAMPLING_RATE,freq,freq*2);
       break;
   case OCTAVE_DOWN :
       // Half the frequency
       freq = OCTAVE_DOWN_APPARENT_FREQ;
       psola.pitchCorrect(outputLeft,OCTAVE_DOWN_SAMPLING_RATE,freq,freq/2);
       psola.pitchCorrect(outputRight,OCTAVE_DOWN_SAMPLING_RATE,freq,freq/2);
       //psola.pitchCorrect(outputLeft,fs_default,freq,freq/2);
       //psola.pitchCorrect(outputRight,fs_default,freq,freq/2);
       break;
   case PITCH_DETECT :
       un.freqF[numOfBufferReads] = flwt.getPitchWithMedian5(outputLeft,BufferLength,PITCH_DETECT_SAMPLING_RATE);
       numOfBufferReads++;
       if(numOfBufferReads >= MaxNumOfBufferReads) {
           serial_send_array(un.freqL,MaxNumOfBufferReads);
           numOfBufferReads = 0;
       }
       break;
   default :
       disp.clear();
       disp.setline(0); disp.print("INVALID");
       disp.setline(1); disp.print("STATUS");
   }
}

/** \brief DMA Interrupt Service Routine
 * 
 * Manage ping-pong input/output buffers.
 * 
 * Calls processData() on input/output buffers.
 * 
 */
interrupt void dmaIsr(void)
{
  unsigned short ifrValue;

  // Variable to indicate that the Input samples are ready for processing
  static unsigned short readyToProcess = 0;

  // Variable to indicate that the processing is completed and that valid
  // data is available "output" buffer
  static unsigned short outputBufAvailable = 0;

  // Variable to switch between the data buffers of the Audio library
  static unsigned short writeBufIndex = 0;

  ifrValue = DMA.getInterruptStatus();
  if ((ifrValue >> DMA_CHAN_ReadR) & 0x01)
  {
    /* Data read from codec is copied to process input buffers.
     Processing is done after configuring DMA for next block of transfer
     ensuring no data loss */
    copyShortBuf(AudioC.audioInLeft[AudioC.activeInBuf],
    InputLeft, BufferLength);
    copyShortBuf(AudioC.audioInRight[AudioC.activeInBuf],
    InputRight, BufferLength);
    readyToProcess = 1;
  }
  else if ((ifrValue >> DMA_CHAN_WriteR) & 0x01)
  {
    if (outputBufAvailable)
    {
      /* Processed buffers need to be copied to audio out buffers as
       audio library is configured for non-loopback mode */
      writeBufIndex = (AudioC.activeOutBuf == FALSE)? TRUE: FALSE;
      copyShortBuf(OutputLeft, AudioC.audioOutLeft[writeBufIndex],
      BufferLength);
      copyShortBuf(OutputRight, AudioC.audioOutRight[writeBufIndex],
      BufferLength);
      outputBufAvailable = 0;
    }
  }

  /* Calling AudioC.isrDma() will copy the buffers to Audio Out of the Codec,
   * initiates next DMA transfers of Audio samples to and from the Codec
   */
  AudioC.isrDma();

  // Check if output buffers are ready. No processing required for write interrupt
  if (readyToProcess)
  {
    readyToProcess = 0;

    processData(InputLeft, InputRight, OutputLeft, OutputRight);

    outputBufAvailable = 1;
  }
}

void parse(int c) {
    const int *input;
    int dataLength;
    switch(c) {
        case 0: // normal mode
            disp.clear();
            disp.setline(0); disp.print("Cmd 0:");
            disp.setline(1); disp.print("NORM MODE");            
            if (CURRENT_STATUS != NORMAL_MODE) {
                AudioC.setupCodec(NORMAL_MODE_SAMPLING_RATE);
            }
            CURRENT_STATUS = NORMAL_MODE;
            break;
        case 1: // phase vocoder
            disp.clear();
            disp.setline(0); disp.print("Cmd 1:");
            disp.setline(1); disp.print("PHASE VOC");   
            if (CURRENT_STATUS != PHASE_VOCODER) {
                AudioC.setupCodec(PHASE_VOCODER_SAMPLING_RATE);
            }
            CURRENT_STATUS = PHASE_VOCODER;
            break;
        case 2: // octave up
            disp.clear();
            disp.setline(0); disp.print("Cmd 2:");
            disp.setline(1); disp.print("OCTAVE UP");   
            if (CURRENT_STATUS != OCTAVE_UP) {
                AudioC.setupCodec(OCTAVE_UP_SAMPLING_RATE);
            }
            CURRENT_STATUS = OCTAVE_UP;
            break;
        case 3: // octave down
            disp.clear();
            disp.setline(0); disp.print("Cmd 3:");
            disp.setline(1); disp.print("OCTAVE DOWN");   
            if (CURRENT_STATUS != OCTAVE_DOWN) {
                AudioC.setupCodec(OCTAVE_DOWN_SAMPLING_RATE);
            }
            CURRENT_STATUS = OCTAVE_DOWN;
            break;
        case 4: // pitch detect
            disp.clear();
            disp.setline(0); disp.print("Cmd 4:");
            disp.setline(1); disp.print("PITCH DETEC");   
            if (CURRENT_STATUS != PITCH_DETECT) {
                AudioC.setupCodec(PITCH_DETECT_SAMPLING_RATE);
            }
            CURRENT_STATUS = PITCH_DETECT;
            numOfBufferReads = 0;
            break;
        default:
            disp.clear();
            disp.setline(0);
            disp.print("Unknown Command");
            CURRENT_STATUS = NORMAL_MODE; // just incase
            break;
    }
}
