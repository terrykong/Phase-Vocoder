#ifndef serial_array_h
#define serial_array_h

/** \file

  This library extends the capabilities of the Energia Serial
  library to send and receive arrays of arbitrary length.
  A command/data class is also defined to manage the sending 
  of integer commands and data over the serial interface.
  
  Usage for array send/receive:
  - Initialize the serial interface
    - DSP Shield: serial_connect()
    - Matlab: [s, connected] = serial_connect(comport, baud);
  - Send data
    - DSP Shield: serial_send_array()
    - Matlab: output = serial_recv_array(s);
  - Receive data
    - DSP Shield: serial_recv_array()
    - Matlab: serial_send_array(s, input);
  - Close communication
    - DSP Shield: do nothing.
    - Matlab: fclose(s);
    Note that if at any time during the communication the
    program encounters an error, the serial buffers between
    the DSP Shield and Matlab might get out of sync preventing
    subsequent communication.  In this case, you should
    manually close the Matlab serial port object by invoking
    the “fclose(s)” command on the Matlab prompt and restarting
    the DSP Shield.
  
  Usage to receive command/data pairs:
  - Initialize the serial interface
    - DSP Shield: serial_connect()
    - Matlab: [s, connected] = serial_connect(comport, baud);
  - Instantiate the SerialCmd class
    - DSP Shield: SerialCmd cmd;
  - Receive command
    - Matlab: serial_cmd(s, cmd, data);
    - SerialCmd::recv();
  - Extract command and data    
    - command = SerialCmd::getCmd();
    - data = SerialCmd::getDataIntPointer();
    
  For more information see documentation for the corresponding functions.

*/

#include "core.h"
#include "OLED.h"

/** \brief Stablish a serial connection
  
  Baud rate must match the baud rate specified in Matlab
  
  \param baudRate specify the serial communication speed
  \return 1 if the connection is suscessfull and 0 otherwise 
*/
int serial_connect(long baudRate);


/** \brief Receive array of type char
  
  \param data specify the pointer to the input array
  \param dataLength is the number of elements in the data receive buffer 
  \return the number of elements actually received 
*/
unsigned int serial_recv_array(char* data, unsigned int dataLength);

/** \brief Receive array of type int
  
  \param data specify the pointer to the input array
  \param dataLength is the number of elements in the data receive buffer 
  \return the number of elements actually received 
*/
unsigned int serial_recv_array(int* data, unsigned int dataLength);

/** \brief Receive array of type long
  
  \param data specify the pointer to the input array
  \param dataLength is the number of elements in the data receive buffer 
  \return the number of elements actually received 
*/
unsigned int serial_recv_array(long* data, unsigned int dataLength);

/** \brief Send array of type char
  
  \param data specify the pointer to the output array
  \param dataLength is the number of elements in the data send buffer 
  \return the number of elements actually sent 
*/
unsigned int serial_send_array(char* data, unsigned int dataLength);

/** \brief Send array of type int
  
  \param data specify the pointer to the output array
  \param dataLength is the number of elements in the data send buffer 
  \return the number of elements actually sent 
*/
unsigned int serial_send_array(int* data, unsigned int dataLength);

/** \brief Send array of type long
  
  \param data specify the pointer to the output array
  \param dataLength is the number of elements in the data send buffer 
  \return the number of elements actually sent 
*/
unsigned int serial_send_array(long* data, unsigned int dataLength);

/** \brief Class to manage reception of commands from the serial interface

  The class creates an internal buffer as an array of type int.
  The data buffer array size must be as large as the maximum array length
  that you expect to receive.
  
  Usage:
  1. Create a SerialCmd instance with the default buffer length:
      SerialCmd cmd();
    or specify the maximum buffer lenght:
      SerialCmd cmd(MAX_BUFFER_LENGTH);
  2. Stablish a serial connection to Matlab with a specific baud rate:
      serial_connect(BAUD_RATE);
  3. When you are ready to receive a command, call the SerialCmd::recv function
      cmd.recv();
    This is a blocking command and it will wait indefinetely.
  4. You can retrieve the command, data array pointer and data array length,
    with the corresponding accessors:
      int command = cmd.getCmd();
      const int*  dataIntPtr  = cmd.getDataIntPointer();
      const long* dataLomgPtr = cmd.getDataLongPointer();
      int dataIntLength  = getDataIntLength();
      int dataLongLength = getDataIntLength();
*/
class SerialCmd
{
    public:
        /// This function receives commands from the serial interface
        unsigned int recv();

        /// Returns the last command received
        inline int getCmd() {return(cmd);}

        /// Returns the number of int elements received
        inline int getDataIntLength() {return(num_int);}

        /// Returns the number of long elements received
        inline int getDataLongLength() {return(num_long);}
        
        /// Returns a pointer to the receive data (int) array
        inline const int*  getDataIntPointer() {return(data_int);}
        
        /// Returns a pointer to the receive data (long) array
        inline const long* getDataLongPointer() {return(data_long);}
        
        /// Default constructor
        SerialCmd();
        
        /// Default constructor
        /// \param size is the length of the internal data (int) array
        SerialCmd(unsigned int size);
        
        /// Destructor
        ~SerialCmd();
        
    private:
        int* buffer;            // Buffer for the received data
        unsigned int buff_size; // Size of the buffer
        int* data_int;          // Buffer of integers received
        unsigned int num_int;   // Number of integers in the buffer
        long* data_long;        // Number of 
        unsigned int num_long;
        int cmd;
        
        // Default size of the buffer
        const unsigned int default_cmd_size = 128;
        
        void init(unsigned int);
};

#endif
