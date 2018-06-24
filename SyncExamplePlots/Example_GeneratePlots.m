clear all; close all; clc;
%% Load
load('Data4Plot.mat');
%% Filter init
SR = 4e6;           % Sample Rate
N = 250;            % Filter order  
Fp_off = 50e3;      % Space off IF
IF1 = 100e3;        % IF freq 1
IF2 = 300e3;        % IF freq 2
Filt_IF_100kHz = fir1(N,[(IF1-Fp_off)/(SR/2),(IF1+Fp_off)/(SR/2)]);
Filt_IF_300kHz = fir1(N,[(IF2-Fp_off)/(SR/2),(IF2+Fp_off)/(SR/2)]);
%% Filter data
data_lf = filtfilt(Filt_IF_100kHz,1,data);
data_hf = filtfilt(Filt_IF_300kHz,1,data);
%% Time sync LF and report delay
[data_sync_lf,delay_lf] = Func_Tsync_Xcorr_FFT(data_lf,SR);
%% Apply this delay, and pre cal cable difference 
data_sync_hf(:,1) = data_hf(:,1);
data_sync_hf(:,2) = circshift(data_hf(:,2),delay_lf + delay_pre_cal);
%% Time & Freq.
t = 1/SR:1/SR:size(data_sync_hf,1)/SR;
N_fft=size(data,1);
fr=(-N_fft/2:N_fft/2-1)*SR/(N_fft)+869e6;
%% Plots
figure(1); clf; hold all; box on; grid on;
plot(fr/1e6,mag2db(abs(fftshift(fft(data)))));
ylim([-40 100]);
ylabel('FFT Magnitude (dBFS)'); xlabel('Frequency (MHz)');
title('FFT of captured data','Interpreter','latex'); 
l = legend('$\textrm{RSP2 1}$','$\textrm{RSP2 2}$','location','northeast');
set(l,'Interpreter','latex'); 
set(findall(gcf,'-property','FontName'),'FontName','TimesNewRoman');
set(findall(gcf,'-property','FontSize'),'FontSize',14);
set(findall(gca, 'Type', 'Line'),'LineWidth',1.5);
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
plot(t,real(data_sync_hf));
ylabel('Normalized Amplitude (V)'); xlabel('Time (s)');
title('Filtered target signal synchronized data','Interpreter','latex'); 
l = legend('$\textrm{RSP2 1}$','$\textrm{RSP2 2}$','location','northeast');
set(l,'Interpreter','latex'); 

subplot(2,2,3)
plot(t(50:150),real(data_sync_hf(50:150,:)));
ylabel('Normalized Amplitude (V)'); xlabel('Time (s)');
title('Filtered target signal synchronized data zoomed at start','Interpreter','latex'); 
l = legend('$\textrm{RSP2 1}$','$\textrm{RSP2 2}$','location','northeast');
set(l,'Interpreter','latex'); 

subplot(2,2,4)
plot(t(end-150:end-50),real(data_sync_hf(end-150:end-50,:)));
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