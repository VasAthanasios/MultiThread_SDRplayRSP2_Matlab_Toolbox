#include "mex.h"
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include "mir_sdr.h"
#include "math.h"
#include <setupAPI.h>

#define buflen 2000000
// if you have n RSP2 changed active_dlls to n
#define active_dlls 2

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

// add more buffers
static short buf_i_0[buflen] = {0};
static short buf_q_0[buflen] = {0};
static short buf_i_1[buflen] = {0};
static short buf_q_1[buflen] = {0};

unsigned int delay[active_dlls] = {0};

static char SDRplay_streaming[active_dlls] = {0};
static char SDRplay_open[active_dlls] = {0};
static char API_loaded[active_dlls] = {0};

// Add more in HMODULE and LPCSTR
HMODULE ApiDll[active_dlls] = {NULL, NULL};
LPCSTR ApiDllName[active_dlls] = {".\\mir_sdr_api_0.dll", ".\\mir_sdr_api_1.dll"};
__declspec(thread) mir_sdr_fnStructT apiFns[active_dlls] = {0};

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
			mexPrintf("SDRplay %d closing... =^(\n",i + 1);
			SDRplay_open[i] = 0;
		}
		if(API_loaded[i])
		{
			FreeLibrary(ApiDll[i]);
			mexPrintf("API DLL %d closing... =^(\n",i + 1);
		}
	}
}

// Gain callbacks
void grCallback0(unsigned int gRdB, unsigned int lnaGRdB, void *cbContext)
{
   return;
}
void grCallback1(unsigned int gRdB, unsigned int lnaGRdB, void *cbContext)
{
   return;
}
// IQ Callbacks
void myCallback0(short *xi, short *xq, unsigned int firstSampleNum, int grChanged, int rfChanged, int fsChanged, unsigned int numSamples, unsigned int reset, unsigned int hwRemoved, void *cbContext)
{
	unsigned int i = 0, ind;
	for(i = 0; i < numSamples; i++) 
	{
		ind = (delay[0]+firstSampleNum+i)%(buflen);
		buf_i_0[ind] = xi[i];
		buf_q_0[ind] = xq[i];
    }
	return;
}
void myCallback1(short *xi, short *xq, unsigned int firstSampleNum, int grChanged, int rfChanged, int fsChanged, unsigned int numSamples, unsigned int reset, unsigned int hwRemoved, void *cbContext)
{
	unsigned int i = 0, ind;
	for(i = 0; i < numSamples; i++) 
	{
		ind = (delay[1]+firstSampleNum+i)%(buflen);
		buf_i_1[ind] = xi[i];
		buf_q_1[ind] = xq[i];
    }
	return;
}

// Add more Callbacks here, and define them above with the corresponding name/buffer
mir_sdr_StreamCallback_t myCallbacks[2] = {myCallback0, myCallback1};
mir_sdr_GainChangeCallback_t grCallbacks[2] = {grCallback0, grCallback1}; 

mxArray * getdata(void)
{
    mxArray *p;
    short	 *re,*im;
    unsigned int i;
	// creates the output matrix
    p=mxCreateNumericMatrix(buflen,active_dlls,mxINT16_CLASS,mxCOMPLEX);
    re=mxGetData(p);
    im=mxGetImagData(p);
    
	for (int col=0; col < buflen; col++) 
	{
		// add extra rows for the extra RSP2
		re[col] 		 = buf_i_0[col];
		re[buflen + col] = buf_i_1[col];
		im[col] 		 = buf_q_0[col];
		im[buflen + col] = buf_q_1[col];
    }

    return p;
}

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]){
	
	char cmd[32];
   
    mexAtExit(closedevice);

	// if argument 
	if(nrhs>0) 
	{   mxGetString(prhs[0],cmd,32); //read command
		// If Data, No need for 2nd input
        if(strcmp("data",cmd)==0)
		{
			plhs[0]=getdata();
			return;
        } 
		// Else Get device index
		if(nrhs<2) {mexErrMsgTxt("Specify device please...");}
		unsigned int api_n = mxGetScalar(prhs[1]);
		api_n = api_n - 1;
		
		if (isnan(api_n)) {mexErrMsgTxt("Device is not a number...");}
		// What was the input cmd?
		// If initialize API
		if (strcmp("init_api",cmd)==0)
		{
			// apiFns[api_n].mir_sdr_DebugEnable_fn(1);
			// Multiple thread stuff
			ApiDll[api_n] = LoadLibrary(ApiDllName[api_n]);
			if (ApiDll[api_n] == NULL)
			{
			  mexPrintf("failed to load api dll '%s'\n", ApiDllName[api_n]);
			  mexPrintf(", error = %d\n", GetLastError());
			}
			mexPrintf("Loaded API dll '%s'!\n", ApiDllName[api_n]);

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
		}
		// else if cmd is set devices
		else if(strcmp(cmd,"get_set_dev")==0) 
		{   			
			mir_sdr_DeviceT devs[active_dlls];
			unsigned int ndev; 
			apiFns[api_n].mir_sdr_DebugEnable_fn(1);
			
			if (apiFns[api_n].mir_sdr_GetDevices_fn(&devs[0], &ndev, 4)!=0) { mexErrMsgTxt("Error in mir_sdr_GetDevices()"); }

			for (int i = 0; i < active_dlls; i++)
			{
				if (devs[i].devAvail == 1)
				{
					if(apiFns[api_n].mir_sdr_SetDeviceIdx_fn(i)!=0) { mexErrMsgTxt("Error in mir_sdr_SetDeviceIdx()"); }
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
		}
		// else if initalize stream
		else if(strcmp("initstream",cmd)==0) 
		{
			if(nrhs<9) {mexErrMsgTxt("Specify device please...");}
			else if(nrhs>9) {mexErrMsgTxt("You have provided more inputs than required...\n");}
			// Init some variables
			mir_sdr_ErrT msg;
			int sampsPerPkt;
			int sysgr;
			// Get the gain reduction
			int gr;
			gr = mxGetScalar(prhs[2]);
			// get the sample rate
			double fs;
			fs = mxGetScalar(prhs[3]);
			// get the frequency
			double fc; 
			fc = mxGetScalar(prhs[4]);
			// get the bandwidth
			mir_sdr_Bw_MHzT bwType;
			bwType = mxGetScalar(prhs[5]);
			// get the IF
			mir_sdr_If_kHzT ifType;
			ifType = mxGetScalar(prhs[6]);
			// get the rspLNA
			int rspLNA;
			rspLNA = mxGetScalar(prhs[7]);
			// get the gain reduction mode
			mir_sdr_SetGrModeT grMode;
			grMode = mxGetScalar(prhs[8]);
			// start the stream finally
			msg = apiFns[api_n].mir_sdr_StreamInit_fn(&gr, fs, fc, bwType, ifType, rspLNA, &sysgr, grMode, &sampsPerPkt, myCallbacks[api_n], grCallbacks[api_n], NULL);
			if (msg == 0) {mexPrintf("SDRplay %d stream initiated successfully!\n",api_n + 1); SDRplay_streaming[api_n] = 1; plhs[0]=mxCreateDoubleScalar(0);}
			else if (msg == 1) {mexPrintf("Other failure mechanism...\n"); plhs[0]=mxCreateDoubleScalar(1);}
			else if (msg == 2) {mexPrintf("Invalid parameters...\n"); plhs[0]=mxCreateDoubleScalar(2);}
			else if (msg == 3) {mexPrintf("Stream initialisation out of range...\n"); plhs[0]=mxCreateDoubleScalar(3);}
			else if (msg == 7) {mexPrintf("Failed to access the device...\n"); plhs[0]=mxCreateDoubleScalar(7);}
			else if (msg == 9) {mexPrintf("SDRplay already initialized...\n"); plhs[0]=mxCreateDoubleScalar(9);}
		}
		// else re initalize stream
		else if(strcmp("reinitstream",cmd)==0) 
		{
			if(nrhs<10) {mexErrMsgTxt("Specify device please...");}
			else if(nrhs>10) {mexErrMsgTxt("You have provided more inputs than required...\n");}
			// Init some variables
			mir_sdr_ErrT msg;
			int sampsPerPkt;
			int sysgr;
			// Get the gain reduction
			int gr;
			gr = mxGetScalar(prhs[2]);
			// get the sample rate
			double fs;
			fs = mxGetScalar(prhs[3]);
			// get the frequency
			double fc; 
			fc = mxGetScalar(prhs[4]);
			// get the bandwidth
			mir_sdr_Bw_MHzT bwType;
			bwType = mxGetScalar(prhs[5]);
			// get the IF
			mir_sdr_If_kHzT ifType;
			ifType = mxGetScalar(prhs[6]);
			// get the LoMode
			mir_sdr_LoModeT loMode;
			loMode = 0;
			// get the rspLNA
			int rspLNA;
			rspLNA = mxGetScalar(prhs[7]);
			// get the gain reduction mode
			mir_sdr_SetGrModeT grMode;
			grMode = mxGetScalar(prhs[8]);
			// get the reason for reinit
			mir_sdr_ReasonForReinitT reasonForReinit;
			reasonForReinit = mxGetScalar(prhs[9]);
			// reinit the stream
			msg = apiFns[api_n].mir_sdr_Reinit_fn(&gr, fs, fc, bwType, ifType, loMode, rspLNA, &sysgr, grMode, &sampsPerPkt, reasonForReinit);
			if (msg == 0) {mexPrintf("SDRplay %d  stream reinitiated successfully!\n", api_n + 1); SDRplay_streaming[api_n] = 1; plhs[0]=mxCreateDoubleScalar(0);}
			else if (msg == 1) {mexPrintf("Other failure mechanism...\n"); }
			else if (msg == 2) {mexPrintf("Invalid parameters...\n"); }
			else if (msg == 3) {mexPrintf("Stream reinitialisation out of range...\n"); }
			else if (msg == 7) {mexPrintf("Failed to access the device...\n"); }
			else if (msg == 8) {mexPrintf("Requested parameters can cause aliasing...\n"); }
		}		
		// else if Port
		else if(strcmp("port",cmd)==0)
		{
			if(nrhs<3) {mexErrMsgTxt("Specify device please...");}
			mir_sdr_RSPII_AntennaSelectT port;
			port = mxGetScalar(prhs[2]);
			if (apiFns[api_n].mir_sdr_RSPII_AntennaControl_fn(port) == 0); 
			{mexPrintf("SDRplay %d port successfully selected!\n",api_n+1);}
		}
		// else if ext_clk
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
		else if (strcmp("delay",cmd)==0)
		{
			unsigned int delay_samps;
			delay_samps = mxGetScalar(prhs[2]);
			delay[api_n] = delay_samps;
			mexPrintf("Delay of %d samples applied on SDRplay %d!\n",delay_samps,api_n+1);
		}
		// else if Stop
		else if(strcmp("streamunint",cmd)==0) 
		{	
			if (apiFns[api_n].mir_sdr_StreamUninit_fn() == 0)
			{
				SDRplay_streaming[api_n] = 0;
				mexPrintf("SDRplay stream off... \n");
			}
			else {mexPrintf("Stream is not on!");}
        } 
		// else if Close
		else if(strcmp("close",cmd)==0) 
		{   
			if (apiFns[api_n].mir_sdr_ReleaseDeviceIdx_fn() == 0)
			{
				SDRplay_open[api_n] = 0;
				mexPrintf("SDRplay closing... =^(\n");
			}
			else {mexPrintf("Stream is not on!");}
        }
		// else wtf u talking about?
		else { mexPrintf("Unknown command: %s\n",cmd);	}
    // Else wrong input bruh    
    } 
	else 
	{
        mexErrMsgTxt("Wrong number of input/output arguments.");
    }
}                                       