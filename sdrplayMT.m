classdef sdrplayMT < handle
    % SDRPLAY construct sdrplay object.
    %
    %   The sdrplay object is a MultiThreaded wrapper for the sdrplay library to
    %   receive directly from multiple RSP2 in matlab. It allows uninterrupted transfers 
    %   without storing signals on disk intermediately.
    %
    %   You can access parameters like Frequency, Sample rate, Bandwidth,
    %   and Gan reduction settings. For the appropriate settings please 
    %   look the specifications. To enable the stream run the function Stream. 
    %   To receive, packet on command you need to provide callback function: 
    %   GetPacket.
    %   The internal buffer of the MEX file is set to 2000000, so depending 
    %   on your sample rate you can access data periodcally. The output data
    %   can be an integer vector in the range [-127, 127], or a single or 
    %   double vector in the range [-1, 1]. By default, data type is double 
    %   and values are in the range [-1, 1]. Alternatively, you can set 
    %   rxNumericType to 'int16'.
    %
    %   Things to be implented in the future, if requested!
    %   Control of Gain Mode.
    %   Control of Lo Mode.
    %   Control of PPM.
    %   Control of DC offset IQ imbalance.
    %   AGC Selection.
    %   BiasT Selection.
    %   AM port Selection.
    %   RF notch Selection.
    %
    %   Vasileiadis Athanasios, 08 11 2018
    %   avasileiadis1@gmail.com
    
    properties (SetObservable, SetAccess=private) 
        DevOpen             % Flag if SDRplay is open.
        DevInfo             % Struct with SDRplay information.
        StreamInit = false; % Flag if stream is initiated.
    end
        
    properties (SetObservable)
        GainReduction   % Set SDRplay gain reduction, based on specification table.
        SampleRateMHz   % Set SDRplay sample rate, 2 - 10 MHz.
        FrequencyMHz    % Set SDRplay tuner frequency, see specification for details.
        BandwidthKHz    % Set SDRplay BandwidthKHz, see table for details.
        IFtype          % Set SDRplay IF to be used, see specification for details.
        LNAstate        % Set SDRplay LNA state based on Grmode, see specification for details.
        Port            % Set SDRplay port, A (default) or B.
        ExtClk          % Set SDRplay external clock state, 0 disabled, 1 enabled.
        PacketData      % SDRplay data packet dropped here.
        StreamInitNumericType='double' % Numeric type of RX samples
        Delay           % Set SDRplay stream delay in samples.
    end
    
    properties (GetAccess=public, SetAccess=private)
        GrMode = 2;     % Set SDRplay gain mode 0 - 2, see specification for details.
    end
    
    methods
        function obj=sdrplayMT(d)
            Open(obj,d)
            obj.GainReduction = 50;     % Set SDRplay gain reduction, based on specification table.
            obj.SampleRateMHz = 2;      % Set SDRplay sample rate, 2 - 10 MHz.
            obj.FrequencyMHz = 10;      % Set SDRplay tuner frequency, see specification for details.
            obj.BandwidthKHz = 600;     % Set SDRplay BandwidthKHz, see table for details.
            obj.IFtype = 0;             % Set SDRplay IF to be used, see specification for details.
            obj.LNAstate = 0;           % Set SDRplay LNA state based on Grmode, see specification for details.
            obj.Port = 'A';             % Set SDRplay Port, A (default) or B.
            obj.ExtClk = 0;             % Set SDRplay External Clock, 0 disabled.
        end

        %% Get dev info when start
        function Open(obj,d)
            % Get devices
            [obj.DevOpen,obj.DevInfo.ndev,obj.DevInfo.SerNo,obj.DevInfo.DevNm,obj.DevInfo.hwVer,obj.DevInfo.DevAvail]=sdrplayMT_mex('get_set_dev',d);
            obj.DevOpen = d;
        end
        %% GainReduction
        function set.GainReduction(obj,g)
            obj.GainReduction = g;
            if obj.StreamInit == true
                obj.ReInit('GR');
            end
        end
        %% LNAstate
        function set.LNAstate(obj,g)
            obj.LNAstate = g;
            if obj.StreamInit == true
                obj.ReInit('GR');
            end
        end
        %% SampleRateMHz
        function set.SampleRateMHz(obj,fs)
            if fs >= 2 && fs <= 10
                obj.SampleRateMHz = fs; ok = 1;
            else 
                warning('Sample rate not correct, please see specifications');
            end
            if obj.StreamInit == true && ok == 1
               obj.ReInit('FS');
            end
        end
        %% FrequencyMHz
        function set.FrequencyMHz(obj,fc)
            if fc >= 1e-3 && fc <= 2e3
            	obj.FrequencyMHz = fc;  ok = 1;
            else
                warning('Tuning frequency not correct, please see specifications');
            end
            if obj.StreamInit == true && ok == 1
                obj.ReInit('RF');
            end
        end
        %% BandwidthKHz
        function set.BandwidthKHz(obj,bw)
            if bw == 200
                obj.BandwidthKHz = bw; ok = 1;
            elseif bw ==  300  
                obj.BandwidthKHz = bw; ok = 1;
            elseif bw == 600
                obj.BandwidthKHz = bw; ok = 1;
            elseif bw == 1536
                obj.BandwidthKHz = bw; ok = 1;
            elseif bw == 5000
                obj.BandwidthKHz = bw; ok = 1;
            elseif bw == 6000
                obj.BandwidthKHz = bw; ok = 1;
            elseif bw == 7000
                obj.BandwidthKHz = bw; ok = 1;
            elseif bw == 8000
                obj.BandwidthKHz = bw; ok = 1;
            else
                warning('Bandwidth not correct, please see specifications');
            end
            if (obj.StreamInit == true &&  ok == 1)
               obj.ReInit('BW');
            end
        end
        %% IFtype
        function set.IFtype(obj,ift)
            if ift == -1
                obj.IFtype = ift; ok = 1;
            elseif ift == 0
                obj.IFtype = ift; ok = 1;
            elseif ift == 450
                obj.IFtype = ift; ok = 1;
            elseif ift == 1620
                obj.IFtype = ift; ok = 1;
            elseif ift == 2048
                obj.IFtype = ift; ok = 1;
            else 
                warning('IF type not correct, please see specifications');
            end
            if (obj.StreamInit == true &&  ok == 1)
                obj.ReInit('IF');
            end
        end
        %% Port
        function set.Port(obj,p)  
            if ~isempty(obj.DevOpen)
                if p == 'A' || p == 'a'
                    obj.Port = 'A';
                    sdrplayMT_mex('port',obj.DevOpen,5);
                elseif p == 'B' || p == 'b'
                    obj.Port = 'B';
                    sdrplayMT_mex('port',obj.DevOpen,6);
                elseif p == 0
                    sdrplayMT_mex('port',obj.DevOpen,0);
                else
                    warning('Port input not correct, please see specifications');
                end
            end
        end
        %% Output Type
        function set.StreamInitNumericType(obj,t)
            if ~isnumerictype(t)
                error('''%s'' is not supported.\nIt would make sense to chose either ''double'', ''single'', or ''int8''.',t);
            end
            obj.StreamInitNumericType=t;
        end
        %% Add delay
        function set.Delay(obj,d)
            if obj.StreamInit
                sdrplayMT_mex('delay',obj.DevOpen,d);
                obj.Delay = obj.Delay + d;
            else
                warning('SDRplay device not streaming!');
            end
        end
        %% Initializes Stream
        function Stream(obj)
            if ~isempty(obj.DevOpen)
                m = sdrplayMT_mex('initstream',obj.DevOpen,obj.GainReduction, obj.SampleRateMHz, obj.FrequencyMHz, ...
                                         obj.BandwidthKHz, obj.IFtype, obj.LNAstate, obj.GrMode);
                if (m == 0)
                    obj.StreamInit=true;
                end
            end
        end
        %% Reinitalisation of the stram ... needs a reason!
        function ReInit(obj,reason)
           r = 0;
           reason = upper(reason);
           if reason == 'GR'
               r = 1;
           elseif reason == 'FS'
               r = 2;
           elseif reason == 'RF'
               r = 4;
           elseif reason == 'BW'
               r = 8;
           elseif reason == 'IF'
               r = 10;
           end
           if r ~= 0
               m = sdrplayMT_mex('reinitstream',obj.DevOpen,obj.GainReduction, obj.SampleRateMHz, obj.FrequencyMHz, ...
                                              obj.BandwidthKHz, obj.IFtype, obj.LNAstate, obj.GrMode, r);
               if m ~= 0
                    Warning('Wrong input, please see specifications and try again!');
               end
           end
        end
        %% Enable Disable external clock
        function set.ExtClk(obj,st)
            if ~isempty(obj.DevOpen) && (st == 0 || st == 1)
                sdrplayMT_mex('ext_clk',obj.DevOpen,st);
                obj.ExtClk = st;
            end
        end
        %% Get packet 
        function [data,samps] = GetPacket(obj)
            if obj.StreamInit
                [data,samps] = sdrplayMT_mex('data');
                if ~isempty(data)
                    data = cast(data,obj.StreamInitNumericType);
                    samps = cast(samps,obj.StreamInitNumericType);
                    if ismember(obj.StreamInitNumericType,{'single' 'double'})
                        data = data - mean(data,1);
                        data=data./32767;
                    end
                end
            else
                warning('SDRplay device not streaming!');
            end
        end

        %% Close SDRplay device
        function Close(obj)
            if ~isempty(obj.DevOpen)
                sdrplayMT_mex('close');
                obj.DevOpen = [];
                obj.StreamInit = [];
            else
                warning('SDRplay device is not open.');
            end
        end            
        %% Stop Stream
        function StopStream(obj)
            if obj.StreamInit
                sdrplayMT_mex('streamunint');
                obj.StreamInit=false;
            end
        end       
    end
  
end
