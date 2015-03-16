% COMPLEX2REALIMAG Converts from complex to real-imag interleaved format. 
%
%   OUTPUT = COMPLEX2REALIMAG(INPUT) converts INPUT vector in complex
%   format to a vector with real and imaginary values interleaved. E.g., if
%   INPUT = [1 + 2 j; 3 + 4 j; 5 + 6 j; 7 + 8 j] the output is
%   OUTPUT = [1; 2; 3; 4; 5; 6; 7; 8]
%
% See also REALIMAG2COMPLEX
function output = complex2RealImag(input)

output = zeros(2 * length(input), 1);
output(1:2:end) = real(input);
output(2:2:end) = imag(input);