%   MultiThreaded SDRPLAY Daisy-Chain TimeSync Example.
%
%   This is a quick demonstration on how to time/phase align RSP2 in Matlab .m editor
%   MySDRplay, sets Frequency, and Bandwidth and enables the external clock. 
%   It then waits for confirmation that the second RSP2 is connected. When
%   the second RSP2 is connected it initializes it with the same
%   parameters, and enables the stream. After it captures the data, it
%   filters the low frequency (anchor signal) and high frequency (target
%   signal). It measures the delay sample difference on the low frequency
%   and applies on the RSP2. Afte that any subsequent frames should have a
%   zero delay sample difference. To check that, a second frame is captured and
%   the delay difference is measured.
%
%   For any questions or assistance you can find me at, 
%   avasileiadis1@sheffield.ac.uk.
%
%   Vasileiadis Athanasios, 08 06 2018
%
clear all; close all; clc;
%% Filter
SR = 4e6;           % Sample Rate
N = 250;            % Filter order  
Fp_off = 50e3;      % Space off IF
IF1 = 100e3;        % IF freq 1
IF2 = 300e3;        % IF freq 2
Filt_IF_100kHz = fir1(N,[(IF1-Fp_off)/(SR/2),(IF1+Fp_off)/(SR/2)]);
Filt_IF_300kHz = fir1(N,[(IF2-Fp_off)/(SR/2),(IF2+Fp_off)/(SR/2)]);
%% Initialize first SDRplay
disp('Make sure only 1 SDRplay is connected ...');
disp('Initiating Daisy Chain, SDRplay connection!');
MySDRplay1 = sdrplayMT(1);
MySDRplay1.FrequencyMHz = 869;
MySDRplay1.BandwidthMHz = 1536;
MySDRplay1.ExtClk(1)        %% Enable clock
disp('Plug in the second SDRplay and press any key!');
pause;
MySDRplay2 = sdrplayMT(2);
MySDRplay2.FrequencyMHz = 869;
MySDRplay2.BandwidthMHz = 1536;
disp('If no errors, Congratz! The two SDRplay share the same clock!');
MySDRplay1.Stream;
MySDRplay2.Stream;
pause(1);
%% Get Data
data = MySDRplay1.GetPacket;
%% Filter data 
data_lf = filtfilt(Filt_IF_100kHz,1,data);
%% Delay on LF
[data_syncd_lf,delay_cal_lf] = Func_Tsync_Xcorr_FFT(data_lf,SR);
%% Apply the delay directly on RSP2
MySDRplay2.Delay(delay_cal_lf);
pause(2);    % Wait abit
%% Get data and measure the delay on HF
data = MySDRplay1.GetPacket;
data_hf = filtfilt(Filt_IF_300kHz,1,data);
%% Delay on HF, if same cabling then, delay_cal_hf = 0
[data_syncd_hf,delay_cal_hf] = Func_Tsync_Xcorr_FFT(data_hf,SR);
%% Plot stuff
t = 1/SR:1/SR:size(data_syncd_lf,1)/SR;
N_fft=size(data,1);
fr=(-N_fft/2:N_fft/2-1)*SR/(N_fft)+869e6;

figure(1); clf; hold all; box on; grid on;
plot(fr/1e6,mag2db(abs(fftshift(fft(data)))));
ylabel('FFT Magnitude (dBFS)'); xlabel('Frequency (MHz)');
title('FFT of captured data','Interpreter','latex'); 
l = legend('$\textrm{RSP2 1}$','$\textrm{RSP2 2}$','location','northeast');
set(l,'Interpreter','latex'); 
set(findall(gcf,'-property','FontName'),'FontName','TimesNewRoman');
set(findall(gcf,'-property','FontSize'),'FontSize',14);
set(findall(gca, 'Type', 'Line'),'LineWidth',0.5);
set(gca,'fontname','times') % Set it to times
h=gcf;
set(h,'PaperOrientation','landscape');
set(h,'PaperUnits','centimeters');
set(h,'outerposition', [0 0 1200 400]);

figure(2); clf; hold all; box on; grid on;
subplot(2,2,1:2)
plot(t,real(data));
ylabel('Normalized Amplitude (V)'); xlabel('Time (s)');
title('Raw unsynchronized data','Interpreter','latex'); 
l = legend('$\textrm{RSP2 1}$','$\textrm{RSP2 2}$','location','northeast');
set(l,'Interpreter','latex'); 

subplot(2,2,3)
plot(t(50:150),real(data(50:150,:)));
ylabel('Normalized Amplitude (V)'); xlabel('Time (s)');
title('Raw unsynchronized data zoomed at start','Interpreter','latex'); 
l = legend('$\textrm{RSP2 1}$','$\textrm{RSP2 2}$','location','northeast');
set(l,'Interpreter','latex'); 

subplot(2,2,4)
plot(t(end-150:end-50),real(data(end-150:end-50,:)));
ylabel('Normalized Amplitude (V)'); xlabel('Time (s)');
title('Raw unsynchronized data zoomed at end','Interpreter','latex'); 
l = legend('$\textrm{RSP2 1}$','$\textrm{RSP2 2}$','location','northeast');
set(l,'Interpreter','latex'); 

set(findall(gcf,'-property','FontName'),'FontName','TimesNewRoman');
set(findall(gcf,'-property','FontSize'),'FontSize',14);
set(findall(gca, 'Type', 'Line'),'LineWidth',.5);
set(gca,'fontname','times') % Set it to times
h=gcf;
set(h,'PaperOrientation','landscape');
set(h,'PaperUnits','centimeters');
set(h,'outerposition', [0 0 1200 800]);

figure(3); clf; hold all; box on; grid on;
subplot(2,2,1:2)
plot(t,real(data_syncd_hf));
ylabel('Normalized Amplitude (V)'); xlabel('Time (s)');
title('Filtered target signal synchronized data','Interpreter','latex'); 
l = legend('$\textrm{RSP2 1}$','$\textrm{RSP2 2}$','location','northeast');
set(l,'Interpreter','latex'); 

subplot(2,2,3)
plot(t(50:150),real(data_syncd_hf(50:150,:)));
ylabel('Normalized Amplitude (V)'); xlabel('Time (s)');
title('Filtered target signal synchronized data zoomed at start','Interpreter','latex'); 
l = legend('$\textrm{RSP2 1}$','$\textrm{RSP2 2}$','location','northeast');
set(l,'Interpreter','latex'); 

subplot(2,2,4)
plot(t(end-150:end-50),real(data_syncd_hf(end-150:end-50,:)));
ylabel('Normalized Amplitude (V)'); xlabel('Time (s)');
title('Filtered target signal synchronized data zoomed at end','Interpreter','latex'); 
l = legend('$\textrm{RSP2 1}$','$\textrm{RSP2 2}$','location','northeast');
set(l,'Interpreter','latex'); 

set(findall(gcf,'-property','FontName'),'FontName','TimesNewRoman');
set(findall(gcf,'-property','FontSize'),'FontSize',14);
set(findall(gca, 'Type', 'Line'),'LineWidth',.5);
set(gca,'fontname','times') % Set it to times
h=gcf;
set(h,'PaperOrientation','landscape');
set(h,'PaperUnits','centimeters');
set(h,'outerposition', [0 0 1200 800]);