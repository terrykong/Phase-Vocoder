#include "serial_array.h"

// Constants for serial communication with Matlab
// Size of header to describe number of bytes being sent
const int header_size = 2;

// Size of the buffer to be used. Must be even
const int buffsize = 16;

// Sync character to be used
char sync_char = 'A';
char char_char = 1;
char int_char = 2;

// Receive a vector of char data from Matlab. Returns the number of integers received
unsigned int serial_recv_array(char* data, unsigned int size)
{
    char val_buff[2];               // Array to hold the header (ints are 2 bytes)
    unsigned int i;                 // LCV
    unsigned int num_vals;          // Number of values read
    int readsize;                   // How many bytes to read
    char nack_str[2] = {0, 0};      // Sent as NACK
  
    // Read in the number of bytes to be received
    Serial.readBytes(val_buff, 2);
  
    // Convert the char array to an integer
    num_vals = (val_buff[0]<<8) | val_buff[1];

    // If we have enough room
    if(num_vals <= size)
    {
        // Acknowledge the size
        Serial.write(val_buff, 2);

        // Loop to receive all the data
        for(i = 0; i < num_vals; i += buffsize)
        {
            // Determine how many bytes to receive. We receive at most
            // buffsize bytes at once, however we can receive less than
            // this during the last iteration since the data size may
            // not be a multiple of buffsize
            readsize = (i + buffsize <= num_vals ? buffsize : num_vals - i);
        
            // Read in the data
            Serial.readBytes(data + i, readsize);
            // ACK data with a sync char. This is just used to make sure
            // that we do not overrun the RX buffer here
            Serial.write(&sync_char, 1);
        }
    }
    // Don't have enough space to receive characters
    else
    {
        // Nack the size. This sends size 0 back, which is not a
        // valid size for data so Matlab will detect the NACK
        Serial.write(nack_str, 2);
        disp.print("No Memory");
        
        num_vals = 0;
    }
    // Returns pointer to the data
    return num_vals; //data;
}

// Receive a vector of int data from Matlab. Returns the number of integers received.
unsigned int serial_recv_array(int* data, unsigned int size)
{
    char data_buff[buffsize];       // Buffer to hold char data
    char char_buff[2];              // Array to hold the header
    unsigned int i, j;              // LCV
    int readsize;                   // How many bytes to read
    char nack_str[2] = {0, 0};      // Sent as NACK
    unsigned int num_char;          // Number of characters read
    unsigned int num_vals;          // Number of values read
  
    // Read in the number of bytes to be received
    Serial.readBytes(char_buff, header_size);
  
    // Convert the char array to an integer
    num_char = (char_buff[0] << 8) | char_buff[1];

    // Divide by two, rounding up
    num_vals = num_char/2 + num_char%2;

    // If we have space to receive the vector
    if(num_vals <= size)
    {
        // Acknowledge the size
        Serial.write(char_buff, 2);
        // Loop to receive all the data
        for(i = 0; i < num_vals; i += buffsize/2)
        {
            // Determine how many bytes to receive. We receive at most
            // buffsize bytes at once, however we can receive less than
            // this during the last iteration since the data size may
            // not be a multiple of buffsize
            readsize = (2*i + buffsize <= num_char ? buffsize : num_char - 2*i);

            // Read in the data
            Serial.readBytes(data_buff, readsize);

            // Now compress to an integer:
            for(j = 0; j < readsize/2; j ++)
            {
                data[i + j] = data_buff[2*j + 1] | (data_buff[2*j] << 8);
            }
            if(readsize%2)
            {
                data[i + readsize/2] = data_buff[i + readsize - 1];
            }
            
            // ACK data with a sync char. This is just used to make sure
            // that we do not overrun the RX buffer here
            Serial.write(&sync_char, 1);
        }
    }
    // Don't have enough space
    else
    {
        // Nack the size. This sends size 0 back, which is not a
        // valid size for data so Matlab will detect the NACK
        Serial.write(nack_str, 2);
        disp.print("No Memory");
        // Indicate the receive failed with return value
        num_vals = 0;
    }

    // Returns pointer to the data
    return num_vals; //data;
}

// Receive an array of long from Matlab
unsigned int serial_recv_array(long* data, unsigned int size)
{
    unsigned int int_vals;  // Number of integer values read
    unsigned int num_vals;  // The number of values read (longs)
    
    // Perform a read on integer values. Note that since there are 2 ints in a long, we
    // can take twice as many int values
    int_vals = serial_recv_array((int*)data, 2*size);
    
    // If we successfully read some data
    if(int_vals)
    {
        // Divide by two, rounding up
        num_vals = int_vals/2 + (int_vals%2);
        
        // If we have an odd number of integer values, make sure the last integer value
        // is zeroed
        if(int_vals%2)
        {
            ((int*)data)[int_vals] = 0;
        }
    }
    // Read failed
    else
    {
        num_vals = 0;
    }
    
    return num_vals;
}

// Write a vector of char data to Matlab. Returns the number of bytes sent
unsigned int serial_send_array(char* data, unsigned int num_vals)
{
    unsigned int i;                // LCV
    char byte_buff[2];             // Number of bytes to send (ints are 2 bytes)
    unsigned int status;           // Returns status of the transmit
  
    // Convert the num_bytes into an array of char's corresponding
    // to the number of bytes to send
    byte_buff[1] = num_vals & 0xFF;
    byte_buff[0] = (num_vals >> 8) & 0xFF;
  
    // Send the header to Matlab
    Serial.write(byte_buff, 2);
    // Send the data to Matlab. Note that we don't need to worry 
    // about overruning Matlab's RX buffer since we ensure it is
    // large enough to never overrun
    status = Serial.write(data, num_vals);
    
    return status;
}

// Write a vector of int data to Matlab. Returns the number of bytes sent. A return of
// zero indicates a failure
unsigned int serial_send_array(int* data, unsigned int num_vals)
{
    unsigned int i;                // LCV
    char byte_buff[2];             // Number of values to send (ints are 2 bytes)
    char data_buff[2];             // Holds uncompressed integer (ints are 2 bytes)
    unsigned int status;           // Returns status of the transmist
  
    // Check that we don't have too many integers
    if(num_vals < 32767)
    {
        // Convert the num_bytes into an array of char's corresponding
        // to the number of bytes to send
        byte_buff[1] = (2*num_vals) & 0xFF;
        byte_buff[0] = ((2*num_vals) >> 8) & 0xFF;
        
        // Send the header to Matlab
        Serial.write(byte_buff, 2);
        
        status = 0;
        
        // Send the data to Matlab. Note that we don't need to worry 
        // about overruning Matlab's RX buffer since we ensure it is
        // large enough to never overrun
        for(i = 0; i < num_vals; i ++)
        {
            // Uncompress the integer
            data_buff[1] = data[i]&0xFF;
            data_buff[0] = (data[i]>>8)&0xFF;
            // Send the individual bytes
            status += Serial.write(data_buff, 2);
        }
    }
    // If we have too many values, indicate a failure in the status
    else
    {
        status = 0;
    }
    return status;
}

// Send an array of long values to Matlab
unsigned int serial_send_array(long* data, unsigned int num_vals)
{
    unsigned int status;    // Number of longs sent
    
    // Cast to long and send the data, noting that we have twice as many ints as longs
    status = serial_send_array((int*)data, 2*num_vals);
    
    // We check that the number of ints sent is even. If it is not even, this indicates
    // some form of error in the transmission of data which we indicate by setting to 0
    if(status%2)
    {
        status = 0;
    }
    // Otherwise, divide by 2 to get the number of longs
    else
    {
        status /= 2;
    }
    
    return status;
}

// Stablish serial connection
int serial_connect(long baud)
{
    int is_connected = 0;   // Indicates if we are connected
    char char_read = NULL;  // Character received
    
    // Initialize the serial port
    Serial.begin(baud);
    // Force DSP Shield to wait indefinitely for Matlab response
    Serial.setTimeout(0);
  
    // Send a sync character to Matlab
    Serial.write(&sync_char, 1);
  
    // Read a character back from Matlab
    Serial.readBytes(&char_read, 1);
  
    // Check if we got the correct sync character back
    is_connected = (char_read == sync_char);
  
    return is_connected;
}

// Initialization function used in the constructors
void SerialCmd::init(unsigned int size)
{
    // Store the size of the buffer
    buff_size = size;
    // Allocate space for the buffer
    buffer = new int[buff_size];
    // Have the int and long data buffers point to the same location
    data_int = buffer;
    data_long = (long*)data_int;
}

// Initialize using the default command list size (in ints)
SerialCmd::SerialCmd()
{
    init(default_cmd_size);
}

// Initialize using an arbitrary command list size (in ints)
SerialCmd::SerialCmd(unsigned int size)
{
    init(size);
}

// Free the allocated memory on destruction
SerialCmd::~SerialCmd()
{
    delete[](buffer);
}

// Receive a command from Matlab
unsigned int SerialCmd::recv()
{
    // Use the cmd variable as a buffer to receive the single int
    serial_recv_array(&cmd, 1);
    // Use the buffer to store the command data
    num_int = serial_recv_array(buffer, buff_size);
    
    // Divide by 2, rounding up
    num_long = num_int/2 + num_int%2;
}

