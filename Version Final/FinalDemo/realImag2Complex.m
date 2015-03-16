% REALIMAG2COMPLEX Converts from real-imag interleaved to complex format. 
%
%   OUTPUT = REALIMAG2COMPLEX(INPUT) converts INPUT vector with interleaved
%   real and imaginary values to complex format. E.g.,
%   INPUT = [1; 2; 3; 4; 5; 6; 7; 8] the output is
%   OUTPUT = [1 + 2 j; 3 + 4 j; 5 + 6 j; 7 + 8 j]
%
% See also COMPLEX2REALIMAG
function output = realImag2Complex(input)

output = input(1:2:end) + j * input(2:2:end);
