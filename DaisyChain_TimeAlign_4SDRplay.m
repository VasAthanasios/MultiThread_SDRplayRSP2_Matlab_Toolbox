%   MultiThreaded SDRPLAY Daisy-Chain TimeSync example.
%
%   This is a quick demonstration on how to time/phase align SDRplay in Matlab .m editor
%   using the class sdrplayMT. It creates an SDRplay object under the name
%   MySDRplay, sets Frequency, and Bandwidth and enables the external clock. 
%   It then waits for confirmation that the second SDRplay is connected. When
%   the second SDRplay is connected it initializes it with the same
%   parameters. When four SDRplay are connected it enables the stream. It then captures 
%   and filetrs modulated data at IF = 100 kHz. 
%   First it measures a coarse delay sample difference using xcorr and applies on each SDRplay. 
%   It then receives another frame of data and performs the final delay offset using FFT and 
%   applies it to each SDRplay. 
%   Subsequent frames are completely time aligned. FFT and a Time plot are
%   presented in the end to prove that the SDRplays are completely aligned.
%
%   For any questions or assistance you can find me at, 
%   avasileiadis1@sheffield.ac.uk.
%
%   Vasileiadis Athanasios, 08 11 2018
%
clear all; close all; clc;
%% Init parameters required for the SDRplays
Fc_MHz = 869;       % Center Frequency.
BW_KHz = 5000;      % Bandwidth.
%% SDRplay 1.
disp('Make sure only 1 SDRplay is connected ...');
disp('Initiating Daisy Chain, SDRplay connection!');
MySDRplay1.FrequencyMHz = Fc_MHz;
MySDRplay1.BandwidthKHz = BW_KHz;MySDRplay1 = sdrplayMT(1);  % Get Set device 1

MySDRplay1.ExtClk = 1;      % Enable clock
disp('Plug in the second SDRplay and press any key!');
pause;
%% SDRplay 2.
MySDRplay2 = sdrplayMT(2);  % Get Set device 2
MySDRplay2.FrequencyMHz = Fc_MHz;
MySDRplay2.BandwidthKHz = BW_KHz;
MySDRplay2.ExtClk = 1;      % Enable clock
disp('Plug in the third SDRplay and press any key!');
pause;
%% SDRplay 3.
MySDRplay3 = sdrplayMT(3);  % Get Set device 3
MySDRplay3.FrequencyMHz = Fc_MHz;
MySDRplay3.BandwidthKHz = BW_KHz;
MySDRplay3.ExtClk = 1;      % Enable clock
disp('Plug in the fourth SDRplay and press any key!');
pause;
%% SDRplay 4.
MySDRplay4 = sdrplayMT(4);  % Get Set device 4
MySDRplay4.FrequencyMHz = Fc_MHz;
MySDRplay4.BandwidthKHz = BW_KHz;
MySDRplay4.ExtClk = 1;      % Enable clock
%% Start the streams!
MySDRplay1.Stream;
MySDRplay2.Stream;
MySDRplay3.Stream;
MySDRplay4.Stream;
disp('If no errors, congratz! The four SDRplay share the same clock! Moving into time alignment');
%% Filter parameters.
SR = MySDRplay1.SampleRateMHz;  % Sample Rate.
N = 250;                        % Filter order.
Fp_off = 50e3;                  % Space off IF.
IF1 = 100e3;                    % IF freq.
Filt_IF_100kHz = fir1(N,[(IF1-Fp_off)/(SR/2),(IF1+Fp_off)/(SR/2)]);
pause(1);                       % Wait abit to fill the buffer... SR/2Meg buffer = 1 s;
%% Get Data and filter it!
data_full = MySDRplay1.GetPacket;
data_lf = filtfilt(Filt_IF_100kHz,1,data_full);
%% Delay on LF Xcorr
[~,delay_xcorr_lf_12] = Func_Tsync_Xcorr(data_lf,[1 2]);
[~,delay_xcorr_lf_13] = Func_Tsync_Xcorr(data_lf,[1 3]);
[~,delay_xcorr_lf_14] = Func_Tsync_Xcorr(data_lf,[1 4]);
%% Apply the Delays!
MySDRplay2.Delay = delay_xcorr_lf_12;
MySDRplay3.Delay = delay_xcorr_lf_13;
MySDRplay4.Delay = delay_xcorr_lf_14;
pause(1);    % Wait abit
%% Get Data and filter it!
data_full = MySDRplay1.GetPacket;
data_lf = filtfilt(Filt_IF_100kHz,1,data_full);
%% Delay on LF FFT 
[~,delay_fft_lf_12] = Func_Tsync_FFT(data_lf,[1 2],SR);
[~,delay_fft_lf_13] = Func_Tsync_FFT(data_lf,[1 3],SR);
[~,delay_fft_lf_14] = Func_Tsync_FFT(data_lf,[1 4],SR);
%% Apply the delay directly
MySDRplay2.Delay = delay_fft_lf_12;
MySDRplay3.Delay = delay_fft_lf_13;
MySDRplay4.Delay = delay_fft_lf_14;
pause(1);    % Wait abit
%% Get Data and filter it!
data_full = MySDRplay1.GetPacket;
data_lf = filtfilt(Filt_IF_100kHz,1,data_full);
%% Plot stuff
t = 1/SR:1/SR:size(data_full,1)/SR;
N_fft=size(data_full,1);
fr=(-N_fft/2:N_fft/2-1)*SR/(N_fft)+869e6;
% FFT
figure(1); clf; hold all; box on; grid on;  ylim([-100,150]);
plot(fr/1e6,mag2db(abs(fftshift(fft(data_full)))));
ylabel('FFT Magnitude (dBFS)'); xlabel('Frequency (MHz)');
title('FFT of captured data','Interpreter','latex'); 
l = legend('$\textrm{SDRplay 1}$','$\textrm{SDRplay 2}$','$\textrm{SDRplay 3}$','$\textrm{SDRplay 4}$','location','northeast');
set(l,'Interpreter','latex'); 
set(findall(gcf,'-property','FontName'),'FontName','TimesNewRoman');
set(findall(gcf,'-property','FontSize'),'FontSize',14);
set(findall(gca, 'Type', 'Line'),'LineWidth',0.5);
set(gca,'fontname','times') % Set it to times
h=gcf;
set(h,'PaperOrientation','landscape');
set(h,'PaperUnits','centimeters');
set(h,'outerposition', [0 0 1200 400]);
% Time
figure(2); clf; hold all; box on; grid on;
subplot(2,2,1:2)
plot(t,real(data_full));
ylabel('Normalized Amplitude (V)'); xlabel('Time (s)');
title('Synchronized data','Interpreter','latex'); 
l = legend('$\textrm{SDRplay 1}$','$\textrm{SDRplay 2}$','$\textrm{SDRplay 3}$','$\textrm{SDRplay 4}$','location','northeast');
set(l,'Interpreter','latex'); 

subplot(2,2,3)
plot(t(50:150),real(data_full(50:150,:)));
ylabel('Normalized Amplitude (V)'); xlabel('Time (s)');
title('Synchronized data zoomed at end','Interpreter','latex'); 
l = legend('$\textrm{SDRplay 1}$','$\textrm{SDRplay 2}$','$\textrm{SDRplay 3}$','$\textrm{SDRplay 4}$','location','northeast');
set(l,'Interpreter','latex'); 

subplot(2,2,4)
plot(t(end-150:end-50),real(data_full(end-150:end-50,:)));
ylabel('Normalized Amplitude (V)'); xlabel('Time (s)');
title('Synchronized data zoomed at end','Interpreter','latex'); 
l = legend('$\textrm{SDRplay 1}$','$\textrm{SDRplay 2}$','$\textrm{SDRplay 3}$','$\textrm{SDRplay 4}$','location','northeast');
set(l,'Interpreter','latex'); 

set(findall(gcf,'-property','FontName'),'FontName','TimesNewRoman');
set(findall(gcf,'-property','FontSize'),'FontSize',14);
set(findall(gca, 'Type', 'Line'),'LineWidth',.5);
set(gca,'fontname','times') % Set it to times
h=gcf;
set(h,'PaperOrientation','landscape');
set(h,'PaperUnits','centimeters');
set(h,'outerposition', [0 0 1200 800]);

%% The end!
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%