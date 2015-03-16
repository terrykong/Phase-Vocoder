% FINAL DEMO interface 
% 
% This script is meant to be used for communication with the DSP chip
%   This opens a serial port to listen (indefinitely) to the DSP chip.
%   
%
function FinalDemo
clear all; clc; close all;

%% Stablish serial connection with DSP Shield
baud = 115200;
% comport = 'COM6';
comport = '/dev/tty.usbserial-SDXX300ZB';

% Connect to the DSP Shield using a handshake
[s, connected] = serial_connect(comport, baud);

% Communication
if connected
    m = 1;
    while m ~= 0
        m = menu('What do you want to do?',...
                 'Normal Mode',...
                 'Phase Vocoder',...
                 'Octave Up',...
                 'Octave Down',...
                 'Pitch Detection (must close ''Stop Collecting!'' window before changing modes');
             
        switch m
            case 1 % Normal mode
                serial_cmd(s, 0, [],'int16');
            case 2 % Phase Vocoder
                serial_cmd(s, 1, [],'int16');
            case 3 % Octave Up
                serial_cmd(s, 2, [],'int16');
            case 4 % Octave Down
                serial_cmd(s, 3, [],'int16');
            case 5 % Pitch Detection
                serial_cmd(s, 4, [],'int16');
                
                % Plotting parameters
                limit = 512;
                pitchBuffer = zeros(1,limit);
                f_han = figure;
                set(f_han,'position',[200,410,1225,800])
                h = plot(pitchBuffer,'linewidth',2);
                hold on;
                plotCScale;
                xlim([0,limit]);
                ylabel('Pitch (Hz)');
                progress = '.';
                progressLim = 10;
                climbing = 1;
                
                % Create the GUI
                S.fh = figure('units','pix',...
                              'pos',[450 400 530 125],...
                              'menubar','none',...              
                              'name','GUI_11',...
                              'numbertitle','off',... 
                              'resize','off');
                S.pb = uicontrol('string','',...
                                 'callback',{},...
                                 'units','pixels',...
                                 'fontsize',55,...
                                 'fontweight','bold',...
                                 'position',[15,10,500,100]);
                set(S.pb,'callback',{@pb_call});
                set(S.pb,'string','Stop Demo!','fontsize',11);  
                set(S.fh,'pos',[0 880 120 50]);
                set(S.pb,'position',[15,10,100,30]);
                drawnow;
                % Collect Data
                while true
                    % Try to collect/send data from the DSP chip
                    lastwarn('');
                    try
                        % Send DSP chip an int to let it know we still want data
                        %fwrite(s,STILL_SEND,'uint8')
                        
                        data = serial_recv_array(s, 'float');
                        % Little animation
                        disp(progress); 
                        figure(f_han);title(progress,'fontweight','bold','fontsize',30);
                        if length(progress) < progressLim && climbing == 1
                            progress = [progress, '.'];
                        elseif length(progress) > 1 && climbing == -1
                            progress(end) = [];
                        elseif length(progress) == progressLim
                            climbing = -1; 
                            progress(end) = [];
                        elseif length(progress) == 1
                            climbing = 1; 
                            progress = [progress, '.'];
                        end
                        % If there was a warning, it's okay. Just catch it
                        if(~isempty(lastwarn))
                            error(lastwarn);
                        end
                        % load it into the pitchBuffer
                        newFront = length(data) + 1;
                        newEnd = limit - length(data) + 1;
                        pitchBuffer( 1:(end-length(data)) ) = pitchBuffer(newFront:end);
                        pitchBuffer( newEnd:end ) = data;
                        set(h,'ydata', pitchBuffer);
                    catch err
                        % do nothing
                    end

                    % After each call check if I should still transmit
                    if ~ishandle(S.fh)  % Check if the figure exists.
                        % So I dont' need to transmit anymore
                        %fwrite(s,DONT_SEND,'char')
                        %break;
                        fclose(s);
                        return;
                    end
                    drawnow;
                end
        end
    end
end

% close port
fclose(s);

    % Helper Functions
    function [] = pb_call(varargin)
        % Callback for pushbutton
        delete(S.fh)  % Delete the figure.
    end

    function plotCScale
        correctpitch = [20;21;23;25;27;28;30;32;33;35;37;39;40;42;44;45;47;49;51;52];
        pitchNames = {'E2','F2','G2','A2','B2','C3','D3','E3','F3','G3','A3','B3','C4','D4',...
                        'E4','F4','G4','A4','B4','C5'};
                    
        pianofreq = @(n) 440*2.^((n-49)/12);
        correctpitch = pianofreq(correctpitch);
        t = (1:limit)';
        one = ones(size(t));
        Y = [];
        X = [];
        for i = 1:length(correctpitch)
            X = [X,t];
            Y = [Y,one*correctpitch(i)];
        end
        plot(X,Y,'r:');
        text(limit*ones(length(correctpitch),1),correctpitch,pitchNames,'fontsize',20,'fontweight','bold');
        ylim([correctpitch(1),correctpitch(end)]);
    end


end



