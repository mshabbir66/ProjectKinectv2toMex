#include "mex.h"
#include "class_handle.hpp"
#include "K4Wv2OpenCVModule.h"
// The class that we are interfacing to
static int initialized = 0;
static mxArray *persistent_array_ptr = NULL;

void exitFcn() {
	if (persistent_array_ptr != NULL)
		mxFree(persistent_array_ptr);
}

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{	
    // Get the command string
    char cmd[64];
	if (nrhs < 1 || mxGetString(prhs[0], cmd, sizeof(cmd)))
		mexErrMsgTxt("First input should be a command string less than 64 characters long.");
        
    // New
    if (!strcmp("new", cmd)) {
        // Check parameters
        if (nlhs != 1)
            mexErrMsgTxt("New: One output expected.");
        // Return a handle to a new C++ instance
        //plhs[0] = convertPtr2Mat<dummy>(new dummy);
		plhs[0] = convertPtr2Mat<CK4Wv2OpenCVModule>(new CK4Wv2OpenCVModule());
        return;
    }
    
    // Check there is a second input, which should be the class instance handle
    if (nrhs < 2)
		mexErrMsgTxt("Second input should be a class instance handle.");
    
    // Delete
    if (!strcmp("delete", cmd)) {
        // Destroy the C++ object
		destroyObject<CK4Wv2OpenCVModule>(prhs[1]);
        // Warn if other commands were ignored
        if (nlhs != 0 || nrhs != 2)
            mexWarnMsgTxt("Delete: Unexpected arguments ignored.");
        return;
    }
    
    // Get the class instance pointer from the second input
	CK4Wv2OpenCVModule *dummy_instance = convertMat2Ptr<CK4Wv2OpenCVModule>(prhs[1]);
    
    // Call the various class methods
    // Train    
    if (!strcmp("InitializeKinectDevice", cmd)) {
        // Check parameters
        if (nlhs < 0 || nrhs < 2)
            mexErrMsgTxt("InitializeKinectDevice: Unexpected arguments.");
        // Call the method
        dummy_instance->InitializeKinectDevice();

		//plhs[0] = mxCreateNumericMatrix(1, 1, mxDOUBLE_CLASS, mxREAL);;
		//double * data = (double *)mxGetData(plhs[0]);
		//data[0] = *dummy_instance->dynamicData;
        return;
    }
    // Test    
    if (!strcmp("UpdateData", cmd)) {
        // Check parameters
        if (nlhs < 0 || nrhs < 2)
            mexErrMsgTxt("UpdateData: Unexpected arguments.");
        // Call the method
        //dummy_instance->UpdateData();
        return;
    }

	if (!strcmp("GrabDepth", cmd)) {
		// Check parameters
		if (nlhs < 0 || nrhs < 2)
			mexErrMsgTxt("GrabDepth: Unexpected arguments.");
		// Call the method
		
		if (!initialized) {
			mexPrintf("MEX-file initializing, creating array\n");
			persistent_array_ptr = mxCreateNumericMatrix(dummy_instance->DEPTH_FRAME_WIDTH, dummy_instance->DEPTH_FRAME_HEIGHT, mxUINT16_CLASS, mxREAL);
			mexMakeArrayPersistent(persistent_array_ptr);
			mexAtExit(exitFcn);
			initialized = 1;
		}
		plhs[0] = persistent_array_ptr;
		UINT16 *dynamicData = (UINT16*)mxGetPr(plhs[0]);
		int nop = dummy_instance->DEPTH_FRAME_HEIGHT* dummy_instance->DEPTH_FRAME_WIDTH;
		dummy_instance->UpdateData(dynamicData);
		//memcpy(dynamicData, dummy_instance->pDepthRAWBuffer, nop*sizeof(UINT16));

		//int nop = dummy_instance->DEPTH_FRAME_HEIGHT* dummy_instance->DEPTH_FRAME_WIDTH;
		//UINT16 *dynamicData = (UINT16*)mxCalloc(nop, sizeof(UINT16));		
		//memcpy(dynamicData, dummy_instance->pDepthRAWBuffer,nop*sizeof(UINT16));
		//plhs[0] = mxCreateNumericMatrix(0, 0, mxUINT16_CLASS, mxREAL);
		//mxSetData(plhs[0], dynamicData);
		//mxSetM(plhs[0], dummy_instance->DEPTH_FRAME_WIDTH);
		//mxSetN(plhs[0], dummy_instance->DEPTH_FRAME_HEIGHT);

		return;
	}
    
    // Got here, so command not recognized
    mexErrMsgTxt("Command not recognized.");
}
