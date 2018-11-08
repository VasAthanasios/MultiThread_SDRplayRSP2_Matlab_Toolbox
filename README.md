!!! Warning !!!

This toolbox contains a precompiled x64 bit MEX for two RSP2 devices. If you you have an issue running the MEX or you have x86 bit version follow these steps:

    1. Copy paste the .dll and .lib from \dll_lib\x86 in the same folder.
    2. Rename them as mir_sdr_api_0 & mir_sdr_api_1. 
    2. Compile the C/MEX file in Matlab with the following command, "mex sdrplay_mex.c mir_sdr_api_0.lib mir_sdr_api_1.lib"

Latest .dll and .lib version can be found at https://www.sdrplay.com/downloads/. **Drivers after 2.11.1 are not compatible with this code.**

!!! Information !!!

The SDRplayMT object is a MultiThreaded wrapper for the SDRplay library to receive directly from multiple RSP2 in matlab. It allows uninterrupted transfers without storing signals on disk intermediately.

This version allows the use of two RSP2. To use more devices you need to add more .dll, .lib files in the folder, do the appropriate changes in the MEX file, and compile it.

You can access parameters like Frequency, Sample rate, Bandwidth, and Gain reduction settings. For the appropriate settings please look at the specifications. To enable the stream, run the command Stream. To receive a packet of data when you want to use callback function: GetPacket. The output data can be an integer vector in the range [-127, 127], or a single or double vector in the range [-1, 1]. By default, the data type is double and values are in the range [-1, 1]. Alternatively, you can set rxNumericType to 'int16'.

The class "sdrplayMT.m" is used with the compiled MEX "sdrplayMT_mex" to communicate with a single RSP2.

The file "sdrplayMT_mex.c" contains C/MEX code that enables the communication between Matlab and the RSP2. This version of C/MEX contains an overlapping buffer. When the buffer is full, it starts writing samples at the start, overwriting the previous samples. A circular buffer will be implemented in the future!

DaisyChain_TimeAlign_Example.m serves the purpose of an example on how to time/phase synchronize two RSP2 devices.

The folder SyncExamplePlots contains the example published in the SDRplay blog.

Features to be implemented in the future if requested! -- Control of Gain Mode -- Control of Lo Mode -- Control of Ppm -- Control of DC offset IQ imbalance -- AGC Selection -- BiasT Selection -- AM port Selection -- RF notch Selection

HUGE thanks to the SDRplay team for their assistance on the API usage!

Big thanks to Tillmann Stübler for his work on the HackRF toolbox.

For any questions or assistance, you can find me at,

avasileiadis1@sheffield.ac.uk
