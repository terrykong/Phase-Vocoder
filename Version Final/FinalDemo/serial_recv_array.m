% SERIAL_RECV_ARRAY This function receives a vector from the DSP Shield. 
%
%   X = SERIAL_RECV_ARRAY(S) receives data vector X over the serial
%     object S assuming INT8 data type.
%  
%   X = SERIAL_RECV_ARRAY(S, TYPE) receives data vector X over the serial
%     object S assuming TYPE as data type.  Valid data types are 'int8',
%     'uint8', 'int16', 'uint16', 'int32' and 'uint32'.
%
% See also SERIAL_CMD, SERIAL_CONNECT, and SERIAL_SEND_ARRAY
function x = serial_recv_array(s, varargin)
    % Pull out the argument type 
    if nargin == 1
        type = 'int8';
    elseif nargin == 2;
        type = varargin{1};
    elseif nargin > 2
        error('Error: Too many arugments in serial_in(s, type)');
    end
    
    % Compute the bytes per value (bpv)
    switch type
        case {'int8', 'uint8'}
            bpv = 1;
        case {'int16', 'uint16'}
            bpv = 2;
        case {'int32', 'uint32'}
            bpv = 4;
        case {'float'}
            bpv = 4;
        otherwise
            error(['Error: valid types are: int8, uint8, int16, ' ...
                   'uint16, int32 and uint32']);
    end
    
    % Wrap in try-catch in case we time out
    try
        % Read in the number of bytes from the DSP Shield
        num_bytes = fread(s, 1, 'uint16');
        % Convert to an integer value
        %num_bytes = bin2dec(reshape(dec2bin(flipud(num_bytes), 8)', 1, []));
       
        % Read in all of the data from the DSP Shield
        x = fread(s, num_bytes/bpv, type);
	% Make sure to close the serial port on error
    catch err
        fclose(s);
        error(err.message);
    end

end