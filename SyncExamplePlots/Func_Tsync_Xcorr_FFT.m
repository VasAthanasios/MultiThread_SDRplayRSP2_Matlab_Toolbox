function [data_sync,delay] = Func_Tsync_Xcorr_FFT(data,SR)
%% Xcorr time sync
[acorr,lags]=xcorr((data(:,1)),(data(:,2)));
[~,Ind]=max((acorr));
lagDiff_xcor = lags(Ind);
%% Apply the delay
data_sync(:,1) = data(:,1);
data_sync(:,2) = circshift(data(:,2),lagDiff_xcor);
%% FFT signal phase sync
NFFT = length(data_sync);
data_fft = fftshift(fft(data_sync,NFFT));
F = (-NFFT/2:NFFT/2-1)*SR/(NFFT);
magnitude_data_fft = abs(data_fft);
[~,max_mag_data_i] = max(magnitude_data_fft);
phase_diff_r = unwrap(angle(data_fft(max_mag_data_i(1),1)/data_fft(max_mag_data_i(2),2)));
%% Find the IF base on the signal
[~,i]=max(mag2db(abs(data_fft)));
TrueIFs = F(i);
%% Round the phase to closest sample
lagDiff_phase =  round(phase_diff_r*SR/(2*pi*TrueIFs(1)));
%% Overall delay
delay = lagDiff_phase+lagDiff_xcor;
%% Apply overall delay
data_sync(:,1) = data(:,1);
data_sync(:,2) = circshift(data(:,2),delay);

end

