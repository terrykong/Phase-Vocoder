% SERIAL_CMD Send a command/data to the DSP Shield. 
%
%   SERIAL_CMD(S, CMD, DATA) send command CMD and DATA over the serial
%     object S assuming INT8 data type.  CMD is an integer.
%  
%   SERIAL_CMD(S, CMD, DATA, TYPE) send command CMD and DATA over the
%     serial object S assuming INT8 data type.  Valid data types are
%     'int8', 'uint8', 'int16', 'uint16', 'int32' and 'uint32'.
%
% This process works as follows:
%   - SERIAL_SEND_ARRAY sends the CMD in 'int16' format.
%   - SERIAL_SEND_ARRAY sends the DATA in TYPE format.
%
% See also SERIAL_CONNECT, SERIAL_SEND_ARRAY, and SERIAL_RECV_ARRAY
function serial_cmd(s, cmd, data, varargin)
    % Extract out the type of the data
    if nargin == 3
        type = 'int8';
    elseif nargin == 4
        type = varargin{1};
    elseif nargin > 5
        error(['Error: Too many arugments in '...
               'serial_cmd(s, cmd, data, type)']);
    end
    
    % Send the command, which is always an int
    serial_send_array(s, cmd, 'int16');
    % Send the data, which can have different types
    serial_send_array(s, data, type);
end