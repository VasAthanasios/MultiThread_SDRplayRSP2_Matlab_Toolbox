function [data_sync,delay] = Func_Tsync_Xcorr(data,dev)
%% Xcorr time sync
[acorr,lags]=xcorr((data(:,dev(1))),(data(:,dev(2))));
[~,Ind]=max(abs(acorr));
delay = lags(Ind);
%% Apply the delay
data_sync(:,1) = data(:,dev(1));
data_sync(:,2) = circshift(data(:,dev(2)),delay);
%% Delete the shift!
data_sync(1:delay,:) = [];
end

