% SERIAL_CONNECT Stablishes a serial communication protocol.
%
% [S, CONNECTED] = SERIAL_CONNECT(COMPORT, BAUD) sets up serial
% communicaiton on COMPORT at given BAUD rate.  Returns the serial object S
% and the CONNECTED status.
%
% The communication protocol is as follows:
%   - The DSP Shield will transmit a character and wait for a reply
%   - If Matlab receives this character and it matches the sync character,
%     Matlab sends the sync character back to the DSP Shield
%   - Once matlab sends the character back, it assumes it is connected to
%     the DSP Shield
%   - Once the DSP Shield receives a sync character back from Matlab, it
%     knows that it is connected to Matlab
%
%   IMPORTANT: if at any time during the communication the program
%   encounters an error, the serial buffers between the DSP Shield and
%   Matlab might get out of sync preventing subsequent communication.
%   In this case, you should manually close the Matlab serial port object
%   by invoking the ?fclose(s)? command on the Matlab prompt and restarting
%   the DSP Shield.
%
% See also SERIAL_CMD, SERIAL_RECV_ARRAY, and SERIAL_SEND_ARRAY
function [s ,connected] = serial_connect(comport, baud)
% Character used to sync the systems
sync_char = 'A';
% Maximum number of attempts to be made to connect
max_attempts = 50;
% Size of header
hsize = 2;

% Indicate whether we have made connection or not
connected = false;

% Open a serial port at desired baud rate
s = serial(comport);
set(s,'BaudRate',baud);
% Make sure buffer is large enough to fit max buffer size
% *** Will be about 64KB for hsize = 2. Shouldn't be an issue on any
% *** modern machine.
set(s, 'InputBufferSize', 256^hsize - 1);
% Use the correct byte order
set(s, 'ByteOrder', 'bigEndian');
% Open the serial port
fopen(s);

% Manually turn of RTS to remove DSP Shield from reset
s.RequestToSend = 'off';

pause(5);

% Wrap in try-catch to prevent serial port from being left open
try
    % Count the number of attempts made
    i = 0;
    % Loop while we are not connected
    while ~connected && i < max_attempts
        % Read in the first byte available on the buffer
        sync = fread(s, 1);
        
        % Increment attempt counter
        i = i + 1;
        
        % Check if we got the sync character
        connected = (sync == sync_char);
    end
    
    % If we got the correct sync character, send the sync char back
    if connected
        fwrite(s, sync_char);
    end

% In event of error, close the serial stream and give a message
catch err
    fclose(s);
    error(err.message);

end

