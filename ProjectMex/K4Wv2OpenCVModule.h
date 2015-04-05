#ifndef __CK4Wv2OpenCVModule_H__
#define __CK4Wv2OpenCVModule_H__
// Default IO
#include <iostream>
// OpenCV
#include <opencv2\opencv.hpp>
#include <opencv2\core\core.hpp>
#include <opencv2\imgproc\imgproc.hpp>

// Kinect for Windows SDK 2.0
#include "Kinect.h"
#include "Kinect.Face.h"


// Namespaces
using namespace std;
using namespace cv;

class  CK4Wv2OpenCVModule
{
	// Sensor frame data values
	// Color frame resolution

	bool provideColor;
	bool provideBody;
	bool provideFace;

	INT64                   m_nStartTime;
	INT64                   timeStamp;

public:
	static const int COLOR_FRAME_WIDTH = 1920;
	static const int COLOR_FRAME_HEIGHT = 1080;
	// Depth frame resolution
	static const int DEPTH_FRAME_WIDTH = 512;
	static const int DEPTH_FRAME_HEIGHT = 424;
	// Infrared frame resolution
	static const int INFRARED_FRAME_WIDTH = 512;
	static const int INFRARED_FRAME_HEIGHT = 424;

	CK4Wv2OpenCVModule() :CK4Wv2OpenCVModule(true, true, true){ }
	CK4Wv2OpenCVModule(bool provideColor, bool provideBody, bool provideFace);
	~CK4Wv2OpenCVModule();

	HRESULT InitializeKinectDevice();

	// Image frame Mat
	Mat colorRAWFrameMat;
	Mat facialAnimationParameters;
	Mat bodyParameters;
	bool faceFlag = FALSE;
	bool bodyFlag = FALSE;

	// Process frame
	void UpdateData(UINT16 *dynamicData);
	INT64 GetTimeStamp();
	void ExtractFaceRotationInDegrees(const Vector4* pQuaternion, float* pPitch, float* pYaw, float* pRoll);
	Point2f CK4Wv2OpenCVModule::BodyToScreen(const CameraSpacePoint& bodyPoint);
	float* bodyBuffer;
	// Calculate Mapped Frame
	ICoordinateMapper*      m_pCoordinateMapper;

	int* facialAnimationParametersBuffer;

	//ushort* pDepthRAWBuffer;
	//ushort* pInfraRAWBuffer;
private:
	// Device
	IKinectSensor* pSensor;
	RGBQUAD* pColorRAWBuffer;




	// Frame reader
	IMultiSourceFrameReader* pMultiSourceFrameReader;
	//Face Readers

	// Face sources
	IFaceFrameSource*		m_pFaceFrameSources[BODY_COUNT];

	// Face readers
	IFaceFrameReader*		m_pFaceFrameReaders[BODY_COUNT];

	// HD Face Sources
	IHighDefinitionFaceFrameSource* m_pHDFaceFrameSources[BODY_COUNT];

	// HDFace readers
	IHighDefinitionFaceFrameReader*		m_pHDFaceFrameReaders[BODY_COUNT];

	// Release function
	template< class T > void SafeRelease(T** ppT);
	void ProcessFaces(IBody** ppBodies);
	void CK4Wv2OpenCVModule::ProcessBody(IBody** ppBodies);
};

#endif

/*
class CK4Wv2OpenCVModule
{

public:
	static const int COLOR_FRAME_WIDTH = 1920;
	static const int COLOR_FRAME_HEIGHT = 1080;

	// Depth frame resolution
	static const int DEPTH_FRAME_WIDTH = 512;
	static const int DEPTH_FRAME_HEIGHT = 424;

	// Infrared frame resolution
	static const int INFRARED_FRAME_WIDTH = 512;
	static const int INFRARED_FRAME_HEIGHT = 424;
	CK4Wv2OpenCVModule();
	~CK4Wv2OpenCVModule();

	HRESULT InitializeKinectDevice();

	// Image frame Mat
	Mat colorRAWFrameMat;
	Mat depthRAWFrameMat;
	Mat infraRAWFrameMat;
	Mat colorMappedFrameMat;
	//Mat depthMappedFrameMat;
	Point2f headPointInDepth;
	Point2f headPointInColor;
	// Process frame
	void UpdateData(UINT16 *dynamicData);

	// Calculate Mapped Frame
	HRESULT calculateMappedFrame();

	ICoordinateMapper* pCoordinateMapper;

	RGBQUAD* pColorRAWBuffer;
	UINT16* pDepthRAWBuffer;
	UINT16* pInfraRAWBuffer;
private:
	// Device
	IKinectSensor* pSensor;


	// Frame reader
	IMultiSourceFrameReader* pMultiSourceFrameReader;

	// Frame data buffers

	// Coordinate
	ColorSpacePoint* pColorCoodinate;
	DepthSpacePoint* pDepthCoordinate;

	// Release function
	template< class T > void SafeRelease(T** ppT);

};
*/

