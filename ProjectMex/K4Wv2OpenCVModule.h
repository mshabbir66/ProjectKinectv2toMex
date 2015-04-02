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


//#ifdef KINECTTWO_EXPORTS
//#define KINECTTWO_API __declspec(dllexport)
//#else
//#define KINECTTWO_API __declspec(dllimport)
//#endif


///*
//#ifdef _DEBUG
//#pragma comment( lib, "opencv_core2410d.lib" )
//#pragma comment( lib, "opencv_highgui2410d.lib" )
//#pragma comment( lib, "opencv_imgproc2410d.lib" )
//#else
//#pragma comment( lib, "opencv_core2410.lib" )
//#pragma comment( lib, "opencv_highgui2410.lib" )
//#pragma comment( lib, "opencv_imgproc2410.lib" )
//#endif
//
//
//#pragma comment( lib, "Kinect20.lib" )
//#pragma comment( lib, "Kinect20.Face.lib" )


// Namespaces
using namespace std;
using namespace cv;


class CK4Wv2OpenCVModule
{
	// Sensor frame data values
	// Color frame resolution





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

#endif