% Spectrum Analysis app interface 
% 
% This script is a companion to the FftMatlabTest.ino sketch, which
% accepts the following serial commands from Matlab:
% cmd (0, input): sends the input array and receives the output array.
%
% Note: we are using Q.15 fixed-point format throughout this lab.
%
clear all; clc; close all;
%% Stablish serial connection with DSP Shield
baud = 115200;
% comport = 'COM6';
comport = '/dev/tty.usbserial-SDXX300ZB';

% Connect to the DSP Shield using a handshake
[s, connected] = serial_connect(comport, baud);

[y,Fs] = audioread('StayTheNight.mp3');
duration = Fs*15;
yy = y(end/2:(end/2+duration),1);

%% Send and receive data
if connected
    %h = msgbox('recording...');
    %pause(3);
    %close(h);
    
    %soundsc(yy,Fs);
    % request the buffer to be filled
    serial_cmd(s, 0, [], 'int16');
    h = msgbox('recording...');
    pause(10);
    close(h);
    % request for the buffer
    serial_cmd(s, 1, [], 'int16');
    
    data = serial_recv_array(s, 'float');
end

% close port
fclose(s);

stairs((1:512)*512/32000,data);
%ylim([0,max(data)]);