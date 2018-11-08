#include "mex.h"
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include "rsp_lib\mir_sdr.h"
#include "math.h"
#include <setupAPI.h>
// If you want to support n SDRplays, changed active_dlls to n.
#define active_dlls 4 
// Buffer Length 2 Meg Samples.
#define buflen 2000000
typedef struct
{
    mir_sdr_StreamInit_t            			mir_sdr_StreamInit_fn;
    mir_sdr_StreamUninit_t          			mir_sdr_StreamUninit_fn;
    mir_sdr_GetDevices_t            			mir_sdr_GetDevices_fn;
    mir_sdr_SetDeviceIdx_t          			mir_sdr_SetDeviceIdx_fn;
    mir_sdr_ReleaseDeviceIdx_t      			mir_sdr_ReleaseDeviceIdx_fn;
    mir_sdr_Reinit_t                			mir_sdr_Reinit_fn;
    mir_sdr_RSPII_AntennaControl_t  			mir_sdr_RSPII_AntennaControl_fn;
	mir_sdr_RSPII_ExternalReferenceControl_t 	mir_sdr_RSPII_ExternalReferenceControl_fn;
	mir_sdr_DebugEnable_t 						mir_sdr_DebugEnable_fn;
} mir_sdr_fnStructT;
// Buffers.
static short buf_i_0[buflen] = {0};
static short buf_q_0[buflen] = {0};
static unsigned int buf_s_0[buflen] = {0};

static short buf_i_1[buflen] = {0};
static short buf_q_1[buflen] = {0};
static unsigned int buf_s_1[buflen] = {0};

static short buf_i_2[buflen] = {0};
static short buf_q_2[buflen] = {0};
static unsigned int buf_s_2[buflen] = {0};

static short buf_i_3[buflen] = {0};
static short buf_q_3[buflen] = {0};
static unsigned int buf_s_3[buflen] = {0};
// Variables 
static char API_loaded[active_dlls] = {0};				// API load flag.
static char SDRplay_open[active_dlls] = {0};			// SDRplay open flag.
static char SDRplay_streaming[active_dlls] = {0};		// SDRplay stream flag.
static char write_flag[active_dlls] = {0};				// Write flag (When get_data == 1) write flag goes down if buffer if is full, and returned to Matlab. (After that write flag goes back up)
static char get_data[active_dlls] = {0};				// Request data (GetPacket), then (get_data == 1).
static unsigned int delay[active_dlls] = {0};			// Add delay on the recorded samples to time sync them.
// Multithread stuff, HMODULE and LPCSTR.
HMODULE ApiDll[active_dlls] = {NULL, NULL, NULL, NULL};
LPCSTR ApiDllName[active_dlls] = {".\\rsp_lib\\mir_sdr_api_0.dll", ".\\rsp_lib\\mir_sdr_api_1.dll", ".\\rsp_lib\\mir_sdr_api_2.dll", ".\\rsp_lib\\mir_sdr_api_3.dll"};
__declspec(thread) mir_sdr_fnStructT apiFns[active_dlls] = {0};
// When MEX is terminated, run this!
void *closedevice(void)
{
	for (int i = 0; i < active_dlls; i++)
	{
		if(SDRplay_streaming[i]) 
		{			
			apiFns[i].mir_sdr_StreamUninit_fn();
			mexPrintf("SDRplay %d stream off... \n",i + 1);
			SDRplay_streaming[i] = 0;
		}
		if(SDRplay_open[i]) 
		{
			apiFns[i].mir_sdr_ReleaseDeviceIdx_fn();
			mexPrintf("SDRplay %d closing... \n",i + 1);
			SDRplay_open[i] = 0;
		}
		if(API_loaded[i])
		{
			FreeLibrary(ApiDll[i]);
			mexPrintf("API DLL %d closing... \n",i + 1);
			API_loaded[i] = 0;
		}
	}
}
// Gain callbacks.
void grCallback0(unsigned int gRdB, unsigned int lnaGRdB, void *cbContext)
{
   return;
}
void grCallback1(unsigned int gRdB, unsigned int lnaGRdB, void *cbContext)
{
   return;
}
void grCallback2(unsigned int gRdB, unsigned int lnaGRdB, void *cbContext)
{
   return;
}
void grCallback3(unsigned int gRdB, unsigned int lnaGRdB, void *cbContext)
{
   return;
}
// IQ Callbacks
void myCallback0(short *xi, short *xq, unsigned int firstSampleNum, int grChanged, int rfChanged, int fsChanged, unsigned int numSamples, unsigned int reset, unsigned int hwRemoved, void *cbContext)
{
	unsigned int ind;
	for (unsigned int i = 0; i < numSamples; i++) 
	{	ind = (delay[0]+firstSampleNum+i)%(buflen);					// Write index based on firstSampleNum and delay[], ind = [0,buflen].
		if (ind == 0 && get_data[0] == 1)  {write_flag[0] = 0;}		// If the buffer is requested and buffer full, stop writing till buffer is returned.
		if (write_flag[0] == 1)										// If write_flag is up, write to buffer.
		{	buf_i_0[ind] = xi[i];
			buf_q_0[ind] = xq[i];
			buf_s_0[ind] = delay[0]+firstSampleNum+i;	}
	}
	return;
}
void myCallback1(short *xi, short *xq, unsigned int firstSampleNum, int grChanged, int rfChanged, int fsChanged, unsigned int numSamples, unsigned int reset, unsigned int hwRemoved, void *cbContext)
{
	unsigned int ind;
	for (unsigned int i = 0; i < numSamples; i++) 
	{	
		ind = (delay[1]+firstSampleNum+i)%(buflen);
		if (ind == 0 && get_data[1] == 1)  {write_flag[1] = 0;}
		if (write_flag[1] == 1)
		{	buf_i_1[ind] = xi[i];
			buf_q_1[ind] = xq[i];
			buf_s_1[ind] = delay[1]+firstSampleNum+i;	}
	}
	return;
}
void myCallback2(short *xi, short *xq, unsigned int firstSampleNum, int grChanged, int rfChanged, int fsChanged, unsigned int numSamples, unsigned int reset, unsigned int hwRemoved, void *cbContext)
{	
	unsigned int ind;
	for (unsigned int i = 0; i < numSamples; i++) 
	{	
		ind = (delay[2]+firstSampleNum+i)%(buflen);
		if (ind == 0 && get_data[2] == 1)  {write_flag[2] = 0;}
		if (write_flag[2] == 1)
		{	buf_i_2[ind] = xi[i];
			buf_q_2[ind] = xq[i];
			buf_s_2[ind] = delay[2]+firstSampleNum+i;	}
	}
	return;
}
void myCallback3(short *xi, short *xq, unsigned int firstSampleNum, int grChanged, int rfChanged, int fsChanged, unsigned int numSamples, unsigned int reset, unsigned int hwRemoved, void *cbContext)
{
	unsigned int ind;
	for (unsigned int i = 0; i < numSamples; i++)
	{	ind = (delay[3]+firstSampleNum+i)%(buflen);
		if (ind == 0 && get_data[3] == 1)  {write_flag[3] = 0;}
		if (write_flag[3] == 1) 
		{	buf_i_3[ind] = xi[i];
			buf_q_3[ind] = xq[i];
			buf_s_3[ind] = delay[3]+firstSampleNum+i;	}
	}
	return;
}
// Group the StreamCallbacks and GainCallbacks.
mir_sdr_StreamCallback_t myCallbacks[active_dlls] = {myCallback0, myCallback1, myCallback2, myCallback3};
mir_sdr_GainChangeCallback_t grCallbacks[active_dlls] = {grCallback0, grCallback1, grCallback2, grCallback3}; 
// When you request data, this is run first!
mxArray * get_iq(void)
{
	// Initialize loop variables.
	unsigned int i, col;
	// Initialize output *p, and both real, imaginary vectors.
    mxArray *p;
    short *re,*im;
	// Create the output matrix.
    p = mxCreateNumericMatrix(buflen,active_dlls,mxINT16_CLASS,mxCOMPLEX);
	// Define which is real which is imaginary.
    re = mxGetData(p);
    im = mxGetImagData(p);
	// Enable get_data flag!
	for (i = 0; i < active_dlls; i++)
	{
		get_data[i] = 1;
	}
	// Wait till all buffers are full.
	while (write_flag[1] && write_flag[2] && write_flag[3] && write_flag[4])
	{
		Sleep(100);
	}
	// Write the buffer to output vectors.
	for (col=0; col < buflen; col++) 
	{
		// Buffer SDRplay1.
		re[col] 		   = buf_i_0[col];
		im[col]	     	   = buf_q_0[col];
		// Buffer SDRplay2.
		re[col + 1*buflen] = buf_i_1[col];
		im[col + 1*buflen] = buf_q_1[col];
		// Buffer SDRplay3.
		re[col + 2*buflen] = buf_i_2[col];
		im[col + 2*buflen] = buf_q_2[col];
		// Buffer SDRplay4.
		re[col + 3*buflen] = buf_i_3[col];
		im[col + 3*buflen] = buf_q_3[col];
	}
	// Return the IQ data!
    return p;
}
// This is to get the buffer sample number!
mxArray * get_s(void)
{
	// Same deal with initalizations.
    mxArray *p;
    unsigned int *fs, i, col;
	// Create the output matrix, this time its mxUINT32_CLASS and mxREAL!
    p = mxCreateNumericMatrix(buflen,active_dlls,mxUINT32_CLASS,mxREAL);
    fs = mxGetData(p);
	// Wait for the write_flag!
	while (write_flag[1] && write_flag[2] && write_flag[3] && write_flag[4])
	{
		Sleep(100);
	}
	// Write the buffers!
	for (col=0; col < buflen; col++) 
	{
		fs[col] 		 	= buf_s_0[col];	
		fs[col + 1*buflen] 	= buf_s_1[col];	
		fs[col + 2*buflen] 	= buf_s_2[col];	
		fs[col + 3*buflen] 	= buf_s_3[col];
	}
	// Change the flags!
	for (i = 0; i < active_dlls; i++)
	{
		get_data[i] = 0;
		write_flag[i] = 1;
	}
	// Return the sample number!
    return p;
}
// Main MEX function!
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]){
	// Initialize the command variable!
	char cmd[32];
    // Make sure you run this when MEX closes up.
    mexAtExit(closedevice);
	// If number of arguments higher than zero. (a command is issued)
	if(nrhs > 0) 
	{   mxGetString(prhs[0], cmd, 32); //read command
		// If Data, no need for 2nd input
        if(strcmp("data", cmd) == 0)
		{
			// Runs the functions to get IQ and sample numbers.
			plhs[0] = get_iq();
			plhs[1] = get_s();
			return;
        } 
		// If number of inputs less than two you havent specified the device.
		if(nrhs < 2) {mexErrMsgTxt("Specify device please...");}
		// Otherwise save the device index.
		unsigned int api_n = mxGetScalar(prhs[1]);
		api_n = api_n - 1;
		// If its not a number then error!
		if (isnan(api_n)) {mexErrMsgTxt("Device is not a number...");}
		// If cmd is get set devices
		if(strcmp(cmd,"get_set_dev")==0) 
		{   			
			// Load APIs.
			ApiDll[api_n] = LoadLibrary(ApiDllName[api_n]);
			if (ApiDll[api_n] == NULL)
			{	mexPrintf("failed to load api dll '%s'\n", ApiDllName[api_n]);
				mexPrintf(", error = %d\n", GetLastError());		}
			apiFns[api_n].mir_sdr_StreamInit_fn						= (mir_sdr_StreamInit_t)           GetProcAddress(ApiDll[api_n], "mir_sdr_StreamInit");
			apiFns[api_n].mir_sdr_StreamUninit_fn					= (mir_sdr_StreamUninit_t)         GetProcAddress(ApiDll[api_n], "mir_sdr_StreamUninit");
			apiFns[api_n].mir_sdr_GetDevices_fn           		    = (mir_sdr_GetDevices_t)           GetProcAddress(ApiDll[api_n], "mir_sdr_GetDevices");
			apiFns[api_n].mir_sdr_SetDeviceIdx_fn         		    = (mir_sdr_SetDeviceIdx_t)         GetProcAddress(ApiDll[api_n], "mir_sdr_SetDeviceIdx");
			apiFns[api_n].mir_sdr_ReleaseDeviceIdx_fn    		    = (mir_sdr_ReleaseDeviceIdx_t)     GetProcAddress(ApiDll[api_n], "mir_sdr_ReleaseDeviceIdx");
			apiFns[api_n].mir_sdr_Reinit_fn               		    = (mir_sdr_Reinit_t)               GetProcAddress(ApiDll[api_n], "mir_sdr_Reinit");
			apiFns[api_n].mir_sdr_RSPII_AntennaControl_fn 		    = (mir_sdr_RSPII_AntennaControl_t) GetProcAddress(ApiDll[api_n], "mir_sdr_RSPII_AntennaControl");
			apiFns[api_n].mir_sdr_RSPII_ExternalReferenceControl_fn  = (mir_sdr_RSPII_ExternalReferenceControl_t) GetProcAddress(ApiDll[api_n], "mir_sdr_RSPII_ExternalReferenceControl");
			apiFns[api_n].mir_sdr_DebugEnable_fn						= (mir_sdr_DebugEnable_t) GetProcAddress(ApiDll[api_n], "mir_sdr_DebugEnable");
			if ((apiFns[api_n].mir_sdr_StreamInit_fn           == NULL)
			   || (apiFns[api_n].mir_sdr_StreamUninit_fn         == NULL)
			   || (apiFns[api_n].mir_sdr_GetDevices_fn           == NULL)
			   || (apiFns[api_n].mir_sdr_SetDeviceIdx_fn         == NULL)
			   || (apiFns[api_n].mir_sdr_ReleaseDeviceIdx_fn     == NULL)
			   || (apiFns[api_n].mir_sdr_Reinit_fn               == NULL)
			   || (apiFns[api_n].mir_sdr_RSPII_AntennaControl_fn == NULL)
			   || (apiFns[api_n].mir_sdr_RSPII_ExternalReferenceControl_fn == NULL)
			   || (apiFns[api_n].mir_sdr_DebugEnable_fn		     == NULL)
			  )
			{
			  mexPrintf("failed to map api dll functions\n");
			  FreeLibrary(ApiDll[api_n]);
			}
			API_loaded[api_n] = 1;
			// Start Set Get process.
			mir_sdr_DeviceT devs[active_dlls];
			unsigned int ndev; 
			// apiFns[api_n].mir_sdr_DebugEnable_fn(1);
			if (apiFns[api_n].mir_sdr_GetDevices_fn(&devs[0], &ndev, active_dlls)!=0) { mexErrMsgTxt("Error in mir_sdr_GetDevices()"); }
			for (int i = 0; i < active_dlls; i++)
			{
				if (devs[i].devAvail == 1)
				{	if(apiFns[api_n].mir_sdr_SetDeviceIdx_fn(i)!=0) { mexErrMsgTxt("Error in mir_sdr_SetDeviceIdx()"); }
					else {mexPrintf("SDRplay device %d selected!\n",i + 1);}
					SDRplay_open[i]=1;
					plhs[0] = mxCreateDoubleScalar(i+1);
					plhs[1] = mxCreateDoubleScalar(ndev);
					plhs[2] = mxCreateString(devs -> SerNo);
					plhs[3] = mxCreateString(devs -> DevNm);
					plhs[4] = mxCreateDoubleScalar(devs -> hwVer);
					plhs[5] = mxCreateDoubleScalar(devs -> devAvail);
					return;
				}
			}
			mexErrMsgTxt("Error, no available devices!");
		}
		// Else if cmd == initalize stream.
		else if(strcmp("initstream",cmd)==0) 
		{
			if(nrhs < 9) {mexErrMsgTxt("Specify device please...");}
			else if(nrhs > 9) {mexErrMsgTxt("You have provided more inputs than required...\n");}
			// Init some variables.
			mir_sdr_ErrT msg;
			int sampsPerPkt;
			int sysgr;
			// Get the gain reduction.
			int gr;
			gr = mxGetScalar(prhs[2]);
			// get the sample rate.
			double fs;
			fs = mxGetScalar(prhs[3]);
			// get the frequency.
			double fc; 
			fc = mxGetScalar(prhs[4]);
			// get the bandwidth
			mir_sdr_Bw_MHzT bwType;
			bwType = mxGetScalar(prhs[5]);
			// get the IF.
			mir_sdr_If_kHzT ifType;
			ifType = mxGetScalar(prhs[6]);
			// get the rspLNA.
			int rspLNA;
			rspLNA = mxGetScalar(prhs[7]);
			// get the gain reduction mode.
			mir_sdr_SetGrModeT grMode;
			grMode = mxGetScalar(prhs[8]);
			// start the stream finally.
			msg = apiFns[api_n].mir_sdr_StreamInit_fn(&gr, fs, fc, bwType, ifType, rspLNA, &sysgr, grMode, &sampsPerPkt, myCallbacks[api_n], grCallbacks[api_n], NULL);
			if (msg == 0) {mexPrintf("SDRplay %d stream initiated successfully!\n",api_n + 1); write_flag[api_n] = 1; SDRplay_streaming[api_n] = 1; plhs[0]=mxCreateDoubleScalar(0);}
			else if (msg == 1) {mexPrintf("Other failure mechanism...\n"); plhs[0]=mxCreateDoubleScalar(1);}
			else if (msg == 2) {mexPrintf("Invalid parameters...\n"); plhs[0]=mxCreateDoubleScalar(2);}
			else if (msg == 3) {mexPrintf("Stream initialisation out of range...\n"); plhs[0]=mxCreateDoubleScalar(3);}
			else if (msg == 7) {mexPrintf("Failed to access the device...\n"); plhs[0]=mxCreateDoubleScalar(7);}
			else if (msg == 9) {mexPrintf("SDRplay already initialized...\n"); plhs[0]=mxCreateDoubleScalar(9);}
		}
		// Else if cmd == re initalize stream.
		else if(strcmp("reinitstream",cmd)==0) 
		{
			if(nrhs < 10) {mexErrMsgTxt("Specify device please...");}
			else if(nrhs>10) {mexErrMsgTxt("You have provided more inputs than required...\n");}
			// Init some variables.
			mir_sdr_ErrT msg;
			int sampsPerPkt;
			int sysgr;
			// Get the gain reduction.
			int gr;
			gr = mxGetScalar(prhs[2]);
			// get the sample rate.
			double fs;
			fs = mxGetScalar(prhs[3]);
			// get the frequency.
			double fc; 
			fc = mxGetScalar(prhs[4]);
			// get the bandwidth.
			mir_sdr_Bw_MHzT bwType;
			bwType = mxGetScalar(prhs[5]);
			// get the IF.
			mir_sdr_If_kHzT ifType;
			ifType = mxGetScalar(prhs[6]);
			// get the LoMode
			mir_sdr_LoModeT loMode;
			loMode = 0;
			// get the rspLNA.
			int rspLNA;
			rspLNA = mxGetScalar(prhs[7]);
			// get the gain reduction mode.
			mir_sdr_SetGrModeT grMode;
			grMode = mxGetScalar(prhs[8]);
			// get the reason for reinit.
			mir_sdr_ReasonForReinitT reasonForReinit;
			reasonForReinit = mxGetScalar(prhs[9]);
			// reinit the stream.
			msg = apiFns[api_n].mir_sdr_Reinit_fn(&gr, fs, fc, bwType, ifType, loMode, rspLNA, &sysgr, grMode, &sampsPerPkt, reasonForReinit);
			if (msg == 0) {mexPrintf("SDRplay %d  stream reinitiated successfully!\n", api_n + 1); write_flag[api_n] = 1;  SDRplay_streaming[api_n] = 1; plhs[0]=mxCreateDoubleScalar(0);}
			else if (msg == 1) {mexPrintf("Other failure mechanism...\n"); }
			else if (msg == 2) {mexPrintf("Invalid parameters...\n"); }
			else if (msg == 3) {mexPrintf("Stream reinitialisation out of range...\n"); }
			else if (msg == 7) {mexPrintf("Failed to access the device...\n"); }
			else if (msg == 8) {mexPrintf("Requested parameters can cause aliasing...\n"); }
		}		
		// Else if cmd == Port.
		else if(strcmp("port",cmd)==0)
		{
			if(nrhs<3) {mexErrMsgTxt("Specify device please...");}
			mir_sdr_RSPII_AntennaSelectT port;
			port = mxGetScalar(prhs[2]);
			if (apiFns[api_n].mir_sdr_RSPII_AntennaControl_fn(port) == 0); 
			{mexPrintf("SDRplay %d port successfully selected!\n",api_n+1);}
		}
		// Else if cmd == ext_clk.
		else if (strcmp("ext_clk",cmd)==0)
		{
			if(nrhs<3) {mexErrMsgTxt("Specify device please...");}
			else if(nrhs>3) {mexErrMsgTxt("You have provided more inputs than required...\n");}
			unsigned int clk_state = mxGetScalar(prhs[2]);
			if (clk_state == 0)
			{
				if (apiFns[api_n].mir_sdr_RSPII_ExternalReferenceControl_fn(0) == 0) 
				{ mexPrintf("External clock of SDRplay %d disabled!\n",api_n+1); }
			}
			else if (clk_state == 1)
			{
				if (apiFns[api_n].mir_sdr_RSPII_ExternalReferenceControl_fn(1) == 0) 
				{ mexPrintf("External clock of SDRplay %d enabled!\n",api_n+1); }
			}
		}
		// Else if cmd == delay.
		else if (strcmp("delay",cmd)==0)
		{
			unsigned int delay_samps;
			delay_samps = mxGetScalar(prhs[2]);
			delay[api_n] = delay[api_n] + delay_samps;
			mexPrintf("Delay of %d samples applied on SDRplay %d!\n",delay_samps,api_n+1);
			mexPrintf("Overall delay on SDRplay %d; %d samples!\n",api_n+1,delay[api_n]);
		}
		// Else if cmd == Stop stream.
		else if(strcmp("streamunint",cmd)==0) 
		{	
			if (apiFns[api_n].mir_sdr_StreamUninit_fn() == 0)
			{
				SDRplay_streaming[api_n] = 0;
				mexPrintf("SDRplay stream off... \n");
			}
			else {mexPrintf("Stream is not on!");}
        } 
		// Else if cmd == Close.
		else if(strcmp("close",cmd)==0) 
		{   
			if (apiFns[api_n].mir_sdr_ReleaseDeviceIdx_fn() == 0)
			{
				SDRplay_open[api_n] = 0;
				mexPrintf("SDRplay closing... =^(\n");
			}
			else {mexPrintf("Stream is not on!");}
        }
		// Else, I do not know this command.
		else { mexPrintf("Unknown command: %s\n",cmd);	}
    // Else wrong number of inputs... 
    } 
	else 
	{
        mexErrMsgTxt("Wrong number of input/output arguments.");
    }
}                                       