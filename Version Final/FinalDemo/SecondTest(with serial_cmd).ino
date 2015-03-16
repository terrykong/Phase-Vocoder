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
#include <dsplib.h>
#include "FLWT.h"

// Globals
const int  BufferLength = 512;

// Declare FLWT object ============
int levels = 6;
int windowLen = BufferLength;
long fs = 32000;
FLWT flwt(levels,windowLen);

const int reqBufLen = 512;
union {
  float freqF[reqBufLen];
  long freqL[reqBufLen];
} 
un;
//float frequencies = new float[reqBufLen];
volatile int freqIndex = 0;
volatile bool freqFlag = false;
// =================================

// Baud rate
const long baudRate = 115200;

// Max serial communications buffer length
const int maxDataLength = 2048;

#pragma DATA_ALIGN(32)
int output[maxDataLength] = {
  0};

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
int LatestLeftBuffer[BufferLength] = {0};
int LatestRightBuffer[BufferLength] = {0};
int LatestLeftBufferFFT[BufferLength*2] = {0};
int LatestRightBufferFFT[BufferLength*2] = {0};
int window[BufferLength];
volatile bool getBufferFlag;

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
  // Intialize flags
  getBufferFlag = false;

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
  AudioC.setupCodec(SAMPLING_RATE_32_KHZ);

  if (status == 0)
  {
    disp.print("Process ON");
    AudioC.attachIntr(dmaIsr);
  }

  // Connect to Matlab
  serial_connect(baudRate);

  // Set audio output volume (0 for line-level input, 100 for a passive microphone)
  AudioC.setOutputVolume(25);
  AudioC.setInputGain(50,50);

  // Do the processing in setup not loop
  int command;

  cmd.recv();
  command = cmd.getCmd();
  parse(command);
  
  cmd.recv();
  command = cmd.getCmd();
  parse(command);
}

/** \brief Main application loop
 * 
 * Do nothing.
 */
void loop()
{
//  int command;
//  cmd.recv();
//  command = cmd.getCmd();
//  parse(command);
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

  // Check if we need to copy or not
  if(getBufferFlag) {
    for(int n = 0; n < BufferLength; n++)
    {
      // Copy a copy of the input into LatestBuffers
      outputLeft[n]  = inputLeft[n]*window[n];
      outputRight[n] = inputRight[n]*window[n];
      LatestLeftBuffer[n] = inputLeft[n];
      LatestRightBuffer[n] = inputRight[n];
    }
    getBufferFlag = false;
  } 
  else {
    for(int n = 0; n < BufferLength; n++)
    {
      outputLeft[n]  = inputLeft[n]*window[n];
      outputRight[n] = inputRight[n]*window[n];
    }
  }


  /*float freq = flwt.getPitch(outputLeft,BufferLength,fs);
  if (freq) {
   Serial.println(freq);
  }*/
  if (freqFlag && freqIndex < reqBufLen) {
    un.freqF[freqIndex] = flwt.getPitch(outputLeft,BufferLength,fs);
    //un.freqF[freqIndex] = (float) freqIndex;
    //un.freqF[freqIndex] = flwt.getPitchLastReliable(outputLeft,BufferLength,fs);
    freqIndex++;
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

/* Deals with the different commands */
void parse(int c) {
  const int *input;
  int dataLength;
  switch(c) {
  case 0: // request that the buffer is filled
    //input = cmd.getDataIntPointer();
    //dataLength = cmd.getDataIntLength();
    disp.clear();
    disp.setline(0);
    disp.print("Cmd 0: ");
    disp.setline(1);
    if(freqFlag) {
      disp.print("STILL FILLING");
    } 
    else {
      freqFlag = true;
      freqIndex = 0;
      disp.print("starting...");
    }
    break;
  case 1: // request for the buffer
    disp.clear();
    disp.setline(0);
    disp.print("Cmd 1: ");
    disp.setline(1);
    if(freqIndex == reqBufLen) {
      serial_send_array(un.freqL,reqBufLen);
      disp.print("SENT");
    } 
    else {
      disp.print("STILL FILLING");
    }
    break;
  default:
    disp.clear();
    disp.setline(0);
    disp.print("Unknown Command");
    break;
  }
}

