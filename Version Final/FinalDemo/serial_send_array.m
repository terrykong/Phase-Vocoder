% SERIAL_SEND_ARRAY Sends a vector of binary data to the DSP Shield. 
%
%   SERIAL_SEND_ARRAY(S, X) send data vector X over the serial
%     object S assuming INT8 data type.
%  
%   SERIAL_SEND_ARRAY(S, X, TYPE) send data vector X over the serial
%     object S assuming TYPE as data type.  Valid data types are 'int8',
%     'uint8', 'int16', 'uint16', 'int32' and 'uint32'.
%
% This process works as follows:
%   - Matlab sends a header telling the DSP Shield how many bytes are in
%   the data vector we are sending
%   - The DSP Shield receives this value and allocates space for the data,
%   then sends back the vector size to acknowledge this.
%   - Matlab receives the size back from the DSP Shield and if it matches
%   the size that it sent, then Matlab begins sending data.
%   - It does this by sending blocks of size buffsize (which should be set
%   to the buffer size in the DSP Shield) and waiting for an acknowledge
%   after each block to know that the buffers have cleared again.
%   - The DSP Shield receives blocks of size buffsize and stores this in
%   the buffer allocated. It then sends back a character letting Matlab
%   know that it is ready for new data.
%   - This continues until the end of the data is reached
%
% See also SERIAL_CMD, SERIAL_CONNECT, and SERIAL_RECV_ARRAY
function serial_send_array(s, x, varargin)
    % Define constants
    buffsize = 16;
    
    % Number of values to send
    num_vals = length(x);
    
    % Indicate if we have a maxlen argument
    lenset = 0;
    
    % Pull out the argument type 
    if nargin == 2
        type = 'int8';
    elseif nargin == 3;
        type = varargin{1};
    elseif nargin == 4;
        type = varargin{1};
        maxlen = varargin{2};
        lenset = 1;
    elseif nargin > 4
        error(['Error: Too many arugments in ' ...
               'serial_out(s, x, type, maxlen)']);
    end
    
    % Extract the bytes per value (bpv) and maxsize for each type
    switch type
        case {'int8', 'uint8'}
            bpv = 1;
            maxsize = 2^16-1;
        case {'int16', 'uint16'}
            maxsize = 2^15-1;
            bpv = 2;
        case {'int32', 'uint32'}
            maxsize = 2^14-1;
            bpv = 4;
        otherwise
            error(['Error: valid types are: int8, uint8, int16, ' ...
                   'uint16, int32 and uint32']);
    end
    
    % If we have a max length argument, overwrite the maxsize
    if lenset
        maxsize = maxlen;
    end
    
    num_bytes = num_vals*bpv;
    
    % Check that our message is not too long
    if num_bytes > maxsize
        error('Error: Message is too long');
    end
    
    % Convert our vector size to two bytes which we send to the DSP Shield
    %bytes = flipud(bin2dec(reshape(dec2bin(num_bytes, 8*hsize), 8, [])'));
    
    % Wrap in try-catch to ensure that we close our stream
    try
        % Send the size of the vector
        fwrite(s, num_bytes, 'uint16');
        % Receive the size of the vector back
        vals_back = fread(s, 1, 'uint16');
        % Check that we have the correct size acknowledge
        if vals_back ~= num_bytes
            error('Error: Bad size acknowledge');
        end
        
        % Send in chunks of no more than buffsize until we reach the end
        for i = 1:buffsize/bpv:num_vals
            fwrite(s, x(i:min(i+buffsize/bpv-1, num_vals)), type);
            % Waits for the acknowledge back
            fread(s, 1);
        end
    % Make sure we close the stream on error
    catch err
        fclose(s);
        error(err.message);
    end
end