function [data_sync,delay] = Func_Tsync_FFT(data,dev,SR)
%% FFT signal phase sync
NFFT = length(data);
data_fft = fftshift(fft([data(:,dev(1)) data(:,dev(2))],NFFT));
F = (-NFFT/2:NFFT/2-1)*SR/(NFFT);
magnitude_data_fft = abs(data_fft);
[~,max_mag_data_i] = max(magnitude_data_fft);
phase_diff_r = unwrap(angle(data_fft(max_mag_data_i(1),1)/data_fft(max_mag_data_i(2),2)));
%% Find the IF base on the signal
[~,i]=max(mag2db(abs(data_fft)));
TrueIFs = F(i);
%% Round the phase to closest sample
delay =  round(phase_diff_r*SR/(2*pi*TrueIFs(1)));
%% Apply delay
data_sync(:,1) = data(:,dev(1));
data_sync(:,2) = circshift(data(:,dev(2)),delay);
%% Delete the shift!
data_sync(1:delay,:) = [];

end

