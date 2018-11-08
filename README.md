# Windows - MEX incompatibility.

This toolbox contains a precompiled Windows x64 MEX for four RSP2 devices. If you you have an issue running the MEX or you have x86 bit version you should recompile the MEX:

	1. Either navigate to \rsp_lib\Install_exe, install SDRplay_RSP_API-Windows-2.11.1 and locate the .dll and .lib files in your file system. Or navigate to \rsp_lib\x86 and copy the .dll and .lib files to \rsp_lib.
    2. Replace all the .dll and .lib files "mir_sdr_api_0.dll", "mir_sdr_api_0.lib", etc... 
    2. Compile the C/MEX file running MEX_Build.m.

# Project information.

This project is a MultiThreaded wrapper for the SDRplay library to configure and receive data directly from multiple RSP2 in Matlab. It allows uninterrupted transfers without storing signals on disk intermediately. 

This version allows the use of four RSP2. To use more devices you need to add more .dll, .lib files in the folder, do appropriate changes in the MEX file, and compile it. If you need any help let me know!

You can access parameters like Frequency, Sample rate, Bandwidth, and Gain reduction settings. For the appropriate settings please look at the specifications. 

## What these files do?

The object "sdrplayMT.m" is used with the compiled MEX "sdrplayMT_mex" to communicate with an RSP2. To load an RSP2 you simply write in Matlab's Command Window "MySDRplay1 = mysdrplay(*n*);", where *n* is the device number.

The file "MEX_Build.m" is used to build a new MEX file, refer to "Windows - MEX incompatibility". 

The file "sdrplayMT_mex.c" contains C/MEX code that enables the communication between Matlab and the RSP2. In case you have issues, refer to "Windows - MEX incompatibility".

"DaisyChain_TimeAlign_Example.m" serves the purpose of showing how to time/phase synchronize two RSP2 devices.

Both "Func_Tsync_Xcorr" and "Func_Tsync_FFT.m" are functions to synchronize the SDRplays that are used in "DaisyChain_TimeAlign_Example.m".

### Implemented functions.

The following functions can be used in the Command Window of Matlab. They can be found inside the object "sdrplayMT.m".

- MySDRplay = mysdrplay(*n*): Attempts to open device with the number *n*. If a device is found and it is availiable it is now opened in the MEX.
- MySDRplay.SampleRateMHz = *fs*: Changes the sample rate of "MySDRplay", where *fs* is in MHz.
- MySDRplay.FrequencyMHz = *fc*: Changes the center frequency of "MySDRplay", where *fc* is in MHz.
- MySDRplay.BandwidthKHz = *bw*: Changes the bandwidth of "MySDRplay", where *bw* is in KHz and the values are predefined (check specifications).
- MySDRplay.IFtype = *if*: Changes the IF type of "MySDRplay", where *if* values are predefined (check specifications).
- MySDRplay.Port = ['A', 'B']: Changes the Port of "MySDRplay".
- MySDRplay.StreamInitNumericType = 'type':  Changes the output type of "MySDRplay".
- MySDRplay.ExtClk = [1, 0]: Enables the external clock of "MySDRplay", 1 is enable, 0 disabled.
- MySDRplay.Stream : Enables the stream of "MySDRplay", if all variables are correct.
- MySDRplay.GetPacket : Returns the buffer for four SDRplays.
- MySDRplay.Delay = *d*: Adds a delay of *d* samples on "MySDRplay". 
- MySDRplay.StopStream : Disables the stream of "MySDRplay".
- MySDRplay.Close : Closes and releases "MySDRplay".

### Not implemented yet.

- Control of Gain Mode 
- Control of Lo Mode 
- Control of Ppm 
- Control of DC offset IQ imbalance 
- AGC Selection 
- BiasT Selection 
- AM port Selection 
- RF notch Selection

# Acknowledgments. 

HUGE thanks to the SDRplay team for their assistance on the API usage!

Big thanks to Tillmann Stübler for his work on the HackRF toolbox.

For any questions or assistance, you can find me at,

avasileiadis1@sheffield.ac.uk