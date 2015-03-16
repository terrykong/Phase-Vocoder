function testingScript
%% Stablish serial connection with DSP Shield
baud = 115200;
% comport = 'COM6';
comport = '/dev/tty.usbserial-SDXX300ZB';

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

% Connect to the DSP Shield using a handshake
[s, connected] = serial_connect(comport, baud);

lastwarn('');
try
    data = serial_recv_array(s, 'float');
    if(~isempty(lastwarn))
        error(lastwarn);
    end
catch err
    err
end

fclose(s);

while true
    if ~ishandle(S.fh)  % Check if the figure exists.
        break;
    end
    drawnow;
end

    % Helper Functions
    function [] = pb_call(varargin)
        % Callback for pushbutton
        delete(S.fh)  % Delete the figure.
    end
    
end