#include "K4Wv2OpenCVModule.h"
#include "mex.h"

// Log tag
static const string TAG = "[K4WOCV]";
static const DWORD c_FaceFrameFeatures =
FaceFrameFeatures::FaceFrameFeatures_BoundingBoxInColorSpace
| FaceFrameFeatures::FaceFrameFeatures_PointsInColorSpace
| FaceFrameFeatures::FaceFrameFeatures_RotationOrientation
| FaceFrameFeatures::FaceFrameFeatures_Happy
| FaceFrameFeatures::FaceFrameFeatures_RightEyeClosed
| FaceFrameFeatures::FaceFrameFeatures_LeftEyeClosed
| FaceFrameFeatures::FaceFrameFeatures_MouthOpen
| FaceFrameFeatures::FaceFrameFeatures_MouthMoved
| FaceFrameFeatures::FaceFrameFeatures_LookingAway
| FaceFrameFeatures::FaceFrameFeatures_Glasses
| FaceFrameFeatures::FaceFrameFeatures_FaceEngagement;

CK4Wv2OpenCVModule::CK4Wv2OpenCVModule(bool provideColor, bool provideBody, bool provideFace) :
provideColor(provideColor),
provideBody(provideBody),
provideFace(provideFace),
pSensor(NULL),
pMultiSourceFrameReader(NULL),
m_pCoordinateMapper(NULL),
m_nStartTime(0),
timeStamp(0)
{
	//cout << TAG << "Initializing Kinect for Windows OpenCV Module." << endl;

	//pSensor = nullptr;
	//pCoordinateMapper = nullptr;
	//pMultiSourceFrameReader = nullptr;

	for (int i = 0; i < BODY_COUNT; i++)
	{
		m_pFaceFrameSources[i] = nullptr;
		m_pFaceFrameReaders[i] = nullptr;
		m_pHDFaceFrameSources[i] = nullptr;
		m_pHDFaceFrameReaders[i] = nullptr;
	}

	// Allocate buffers
	//pColorRAWBuffer = new RGBQUAD[COLOR_FRAME_WIDTH * COLOR_FRAME_HEIGHT];
	//pDepthRAWBuffer = new ushort[DEPTH_FRAME_WIDTH * DEPTH_FRAME_HEIGHT];
	//pInfraRAWBuffer = new ushort[INFRARED_FRAME_WIDTH * INFRARED_FRAME_HEIGHT];


	// Set 0
	//memset(pColorRAWBuffer, 0, COLOR_FRAME_WIDTH * COLOR_FRAME_HEIGHT * sizeof(RGBQUAD));
	//memset( pColorRAWBuffer, 0, DEPTH_FRAME_WIDTH * DEPTH_FRAME_HEIGHT * sizeof( ushort ) );
	//memset( pColorRAWBuffer, 0, INFRARED_FRAME_WIDTH * INFRARED_FRAME_HEIGHT * sizeof( ushort ) );

	// Set Mat
	//colorRAWFrameMat = Mat(Size(COLOR_FRAME_WIDTH, COLOR_FRAME_HEIGHT), CV_8UC4, (void*)pColorRAWBuffer);
	//depthRAWFrameMat = Mat( Size( DEPTH_FRAME_WIDTH, DEPTH_FRAME_HEIGHT ), CV_16UC1, (void*)pDepthRAWBuffer );
	//infraRAWFrameMat = Mat( Size( INFRARED_FRAME_WIDTH, INFRARED_FRAME_HEIGHT ), CV_16UC1, (void*)pInfraRAWBuffer );

	if (provideFace)
	{
		facialAnimationParametersBuffer = new UINT16[FaceProperty::FaceProperty_Count];
		memset(facialAnimationParametersBuffer, 0, (FaceProperty::FaceProperty_Count) * sizeof(UINT16));
		//facialAnimationParameters = Mat(Size((FaceProperty::FaceProperty_Count), 1), CV_32F, (void*)facialAnimationParametersBuffer);
	}

	if (provideBody)
	{
		bodyBuffer = new float[7 * JointType_Count];
		bodyParameters = Mat(Size((7 * JointType_Count), 1), CV_32F, (void*)bodyBuffer);
	}


}

CK4Wv2OpenCVModule::~CK4Wv2OpenCVModule()
{
	//cout << TAG << "Releasing Kinect for Windows OpenCV Module." << endl;
	// Release buffers
	if (pColorRAWBuffer)
	{
		delete pColorRAWBuffer;
		pColorRAWBuffer = nullptr;
	}

	if (pSensor)
	{
		pSensor->Close();
		SafeRelease(&pSensor);
	}
	for (int i = 0; i < BODY_COUNT; i++)
	{
		SafeRelease(&m_pFaceFrameSources[i]);
		SafeRelease(&m_pFaceFrameReaders[i]);
		SafeRelease(&m_pHDFaceFrameSources[i]);
		SafeRelease(&m_pHDFaceFrameReaders[i]);
	}

}

HRESULT CK4Wv2OpenCVModule::InitializeKinectDevice()
{
	HRESULT hr;

	// Get default sensor
	hr = GetDefaultKinectSensor(&pSensor);
	if (FAILED(hr))
	{
		cerr << TAG << "Sensor initialization error at - " << __FUNCTIONW__ << endl;
		return hr;
	}

	if (pSensor)
	{
		// Get coordinate mapper
		// Open sensor
		hr = pSensor->Open();
		if (SUCCEEDED(hr))
		{
			char frameFlg = 0;

			if (provideColor)
				frameFlg |= FrameSourceTypes_Color;
			if (provideBody)
				frameFlg |= FrameSourceTypes_Body;
			

			//frameFlg = FrameSourceTypes_Infrared | FrameSourceTypes_Color | FrameSourceTypes_Depth | FrameSourceTypes_Body;

			hr = pSensor->OpenMultiSourceFrameReader(frameFlg, &pMultiSourceFrameReader);
		}

		if (SUCCEEDED(hr))
		{
			hr = pSensor->get_CoordinateMapper(&m_pCoordinateMapper);

		}

		if (SUCCEEDED(hr))
		{
			// create a face frame source + reader to track each body in the fov
			for (int i = 0; i < BODY_COUNT; i++)
			{
				if (SUCCEEDED(hr))
				{
					// create the face frame source by specifying the required face frame features
					hr = CreateFaceFrameSource(pSensor, 0, c_FaceFrameFeatures, &m_pFaceFrameSources[i]);
					hr = CreateHighDefinitionFaceFrameSource(pSensor, &m_pHDFaceFrameSources[i]);

				}
				if (SUCCEEDED(hr))
				{
					// open the corresponding reader
					hr = m_pFaceFrameSources[i]->OpenReader(&m_pFaceFrameReaders[i]);
					hr = m_pHDFaceFrameSources[i]->OpenReader(&m_pHDFaceFrameReaders[i]);
				}
			}
		}

		//
		IAudioSource* pAudioSource = NULL;

		if (SUCCEEDED(hr))
		{
			hr = pSensor->get_AudioSource(&pAudioSource);

		}
		if (SUCCEEDED(hr))
		{
			hr = pAudioSource->OpenReader(&m_pAudioBeamFrameReader);
		}
	}

	if (!pSensor || FAILED(hr))
	{
		cerr << TAG << "No devices ready." << endl;
		return E_FAIL;
	}

	return hr;
}

void CK4Wv2OpenCVModule::UpdateData(UINT16 *dynamicData)
{
	UpdateData();
	memcpy(dynamicData, facialAnimationParametersBuffer, (FaceProperty::FaceProperty_Count) * sizeof(UINT16));

}
void CK4Wv2OpenCVModule::UpdateData()
{
	if (!pMultiSourceFrameReader)
		return;
	faceFlag = FALSE;
	bodyFlag = FALSE;
	IMultiSourceFrame* pMultiSourceFrame = nullptr;
	IDepthFrame* pDepthFrame = nullptr;
	IColorFrame* pColorFrame = nullptr;
	IInfraredFrame* pInfraFrame = nullptr;
	IBodyFrame* pBodyFrame = nullptr;

	HRESULT hr = pMultiSourceFrameReader->AcquireLatestFrame(&pMultiSourceFrame);

	if (SUCCEEDED(hr))
	{
		// Receive color
		IColorFrameReference* pColorFrameReference = nullptr;

		HRESULT hrColor = pMultiSourceFrame->get_ColorFrameReference(&pColorFrameReference);
		pColorFrameReference->get_RelativeTime(&timeStamp);

		if (!m_nStartTime)
		{
			m_nStartTime = timeStamp;
		}

		if (provideColor)
		{
			if (SUCCEEDED(hrColor))
			{
				hrColor = pColorFrameReference->AcquireFrame(&pColorFrame);
				//pColorFrame->get_RelativeTime(&timeStamp);

			}

			if (SUCCEEDED(hrColor))
			{
				unsigned int colorBufferSize = COLOR_FRAME_WIDTH * COLOR_FRAME_HEIGHT * sizeof(RGBQUAD);
				//hr = pColorFrame->CopyConvertedFrameDataToArray(colorBufferSize, reinterpret_cast<BYTE*>(pColorRAWBuffer), ColorImageFormat_Bgra);

			}
		}

		SafeRelease(&pColorFrameReference);

		//
		//

		if (provideBody || provideFace)
		{


			IBodyFrameReference* pBodyFrameReference = nullptr;
			HRESULT hrBody = pMultiSourceFrame->get_BodyFrameReference(&pBodyFrameReference);
			IBody* ppBodies[BODY_COUNT] = { 0 };

			if (SUCCEEDED(hrBody))
			{
				hrBody = pBodyFrameReference->AcquireFrame(&pBodyFrame);
			}
			SafeRelease(&pBodyFrameReference);


			if (SUCCEEDED(hrBody))
			{
				hr = pBodyFrame->GetAndRefreshBodyData(BODY_COUNT, ppBodies);
			}
			if (SUCCEEDED(hrBody))
			{
				if (provideFace)
					ProcessFaces(ppBodies);
				if (provideBody)
					ProcessBody(ppBodies);

				for (int i = 0; i < _countof(ppBodies); ++i)
				{
					SafeRelease(&ppBodies[i]);
				}
			}

		}

		SafeRelease(&pColorFrame);
		SafeRelease(&pBodyFrame);
		SafeRelease(&pMultiSourceFrame);

		IAudioBeamFrameArrivedEventArgs* pAudioBeamFrameArrivedEventArgs = NULL;
		IAudioBeamFrameReference* pAudioBeamFrameReference = NULL;
		IAudioBeamFrameList* pAudioBeamFrameList = NULL;
		IAudioBeamFrame* pAudioBeamFrame = NULL;
		UINT32 subFrameCount = 0;

		hr = m_pAudioBeamFrameReader->GetFrameArrivedEventData(m_hFrameArrivedEvent, &pAudioBeamFrameArrivedEventArgs);

		if (SUCCEEDED(hr))
		{
			hr = pAudioBeamFrameArrivedEventArgs->get_FrameReference(&pAudioBeamFrameReference);
		}

		if (SUCCEEDED(hr))
		{
			hr = pAudioBeamFrameReference->AcquireBeamFrames(&pAudioBeamFrameList);
		}

		if (SUCCEEDED(hr))
		{
			// Only one audio beam is currently supported
			hr = pAudioBeamFrameList->OpenAudioBeamFrame(0, &pAudioBeamFrame);
		}

		if (SUCCEEDED(hr))
		{
			hr = pAudioBeamFrame->get_SubFrameCount(&subFrameCount);
		}

		if (SUCCEEDED(hr) && subFrameCount > 0)
		{
			for (UINT32 i = 0; i < subFrameCount; i++)
			{
				// Process all subframes
				IAudioBeamSubFrame* pAudioBeamSubFrame = NULL;

				hr = pAudioBeamFrame->GetSubFrame(i, &pAudioBeamSubFrame);

				if (SUCCEEDED(hr))
				{
					//ProcessAudio(pAudioBeamSubFrame);
					mexPrintf("Success\n");
				}

				SafeRelease(&pAudioBeamSubFrame);
			}
		}

		SafeRelease(&pAudioBeamFrame);
		SafeRelease(&pAudioBeamFrameList);
		SafeRelease(&pAudioBeamFrameReference);
		SafeRelease(&pAudioBeamFrameArrivedEventArgs);

	}
}

void CK4Wv2OpenCVModule::ProcessFaces(IBody** ppBodies){


	for (int iFace = 0; iFace < BODY_COUNT; ++iFace)
	{
		// retrieve the latest face frame from this reader
		IFaceFrame* pFaceFrame = nullptr;
		HRESULT hr = m_pFaceFrameReaders[iFace]->AcquireLatestFrame(&pFaceFrame);

		BOOLEAN bFaceTracked = false;
		if (SUCCEEDED(hr) && nullptr != pFaceFrame)
		{
			// check if a valid face is tracked in this face frame
			hr = pFaceFrame->get_IsTrackingIdValid(&bFaceTracked);
		}

		if (SUCCEEDED(hr))
		{
			if (bFaceTracked)
			{
				IFaceFrameResult* pFaceFrameResult = nullptr;
				RectI faceBox = { 0 };
				PointF facePoints[FacePointType::FacePointType_Count];
				Vector4 faceRotation;
				DetectionResult faceProperties[FaceProperty::FaceProperty_Count];

				hr = pFaceFrame->get_FaceFrameResult(&pFaceFrameResult);

				// need to verify if pFaceFrameResult contains data before trying to access it
				if (SUCCEEDED(hr) && pFaceFrameResult != nullptr)
				{

					hr = pFaceFrameResult->GetFaceProperties(FaceProperty::FaceProperty_Count, faceProperties);
					for (int i = 0; i < FaceProperty::FaceProperty_Count; i++)
					{
						facialAnimationParametersBuffer[i] = (int)faceProperties[i];
						//std::cout << to_string(faceProperties[i]) << std::endl;
					}
				}

				SafeRelease(&pFaceFrameResult);

				iFace = BODY_COUNT;
				faceFlag = TRUE;
			}
			else
			{
				// face tracking is not valid - attempt to fix the issue
				// a valid body is required to perform this step
				IBody* pBody = ppBodies[iFace];
				if (pBody != nullptr)
				{
					BOOLEAN bTracked = false;
					hr = pBody->get_IsTracked(&bTracked);

					UINT64 bodyTId;
					if (SUCCEEDED(hr) && bTracked)
					{
						// get the tracking ID of this body
						hr = pBody->get_TrackingId(&bodyTId);
						if (SUCCEEDED(hr))
						{
							// update the face frame source with the tracking ID
							m_pFaceFrameSources[iFace]->put_TrackingId(bodyTId);
						}
					}
				}
			}

		}
		SafeRelease(&pFaceFrame);

	}

	//for (int iFace = 0; iFace < BODY_COUNT; ++iFace){
	//	IBody* pBody = ppBodies[iFace];
	//	if (pBody != nullptr)
	//	{
	//		BOOLEAN bTracked = false;
	//		HRESULT hr = pBody->get_IsTracked(&bTracked);
	//		UINT64 bodyTId;
	//		if (SUCCEEDED(hr) && bTracked)
	//		{
	//			// get the tracking ID of this body
	//			hr = pBody->get_TrackingId(&bodyTId);
	//			if (SUCCEEDED(hr))
	//			{
	//				// update the face frame source with the tracking ID
	//				m_pFaceFrameSources[iFace]->put_TrackingId(bodyTId);

	//			}
	//			IFaceFrame * pFaceFrame = nullptr;
	//			hr = m_pFaceFrameReaders[iFace]->AcquireLatestFrame(&pFaceFrame);

	//			//::cout << "HD Face frame acquired\n";

	//			BOOLEAN bFaceTracked = false;
	//			if (SUCCEEDED(hr) && nullptr != pFaceFrame)
	//			{
	//				// check if a valid face is tracked in this face frame
	//				hr = pFaceFrame->get_IsTrackingIdValid(&bFaceTracked);
	//				//pHDFaceFrame->get_RelativeTime(&T);
	//				//TRACE(L"Time: %d\n", T);
	//				IFaceFrameResult* pFaceFrameResult = nullptr;
	//				DetectionResult faceProperties[FaceProperty::FaceProperty_Count];

	//				if (SUCCEEDED(hr))
	//					hr = pFaceFrame->get_FaceFrameResult(&pFaceFrameResult);

	//				float* pAnimationUnits;//= new float[FaceShapeAnimations_Count];
	//				pAnimationUnits = facialAnimationParametersBuffer;

	//				if (SUCCEEDED(hr))
	//				{
	//					hr = pFaceFrameResult->GetFaceProperties(FaceProperty::FaceProperty_Count, faceProperties);
	//					cout << "success\n";
	//				}

	//				//std::cout << "Yay\n";
	//				//for (int i = 0; i < FaceShapeAnimations_Count; i++)
	//					//std::cout << to_string(pAnimationUnits[i])<<std::endl;
	//				//delete[] pAnimationUnits;
	//				SafeRelease(&pFaceFrameResult);
	//				iFace = BODY_COUNT;
	//				faceFlag = TRUE;
	//			}
	//			
	//			
	//		}

	//	}
	//}

}


template< class T > void CK4Wv2OpenCVModule::SafeRelease(T** ppT)
{
	if (*ppT)
	{
		(*ppT)->Release();
		*ppT = nullptr;
	}
}

INT64 CK4Wv2OpenCVModule::GetTimeStamp(){
	return timeStamp - m_nStartTime;

}

void CK4Wv2OpenCVModule::ProcessBody(IBody** ppBodies)
{
	for (int iFace = 0; iFace < BODY_COUNT; ++iFace)
	{
		IBody* pBody = ppBodies[iFace];
		if (pBody)
		{
			BOOLEAN bTracked = false;
			HRESULT hr = pBody->get_IsTracked(&bTracked);

			if (SUCCEEDED(hr) && bTracked)
			{
				Joint joints[JointType_Count];
				hr = pBody->GetJoints(JointType_Count, joints);

				JointOrientation jOs[JointType_Count];
				hr = pBody->GetJointOrientations(JointType_Count, jOs);

				for (int i = 0; i < JointType_Count; i++)
				{
					bodyBuffer[7 * i] = (float)joints[i].Position.X;
					bodyBuffer[7 * i + 1] = (float)joints[i].Position.Y;
					bodyBuffer[7 * i + 2] = (float)joints[i].Position.Z;
					bodyBuffer[7 * i + 3] = (float)jOs[i].Orientation.w;
					bodyBuffer[7 * i + 4] = (float)jOs[i].Orientation.x;
					bodyBuffer[7 * i + 5] = (float)jOs[i].Orientation.y;
					bodyBuffer[7 * i + 6] = (float)jOs[i].Orientation.z;

				}
				iFace = BODY_COUNT;
				bodyFlag = 1;

			}
		}


	}
}



void CK4Wv2OpenCVModule::ExtractFaceRotationInDegrees(const Vector4* pQuaternion, float* pPitch, float* pYaw, float* pRoll)
{
	double x = pQuaternion->x;
	double y = pQuaternion->y;
	double z = pQuaternion->z;
	double w = pQuaternion->w;

	// convert face rotation quaternion to Euler angles in degrees		
	//double dPitch, dYaw, dRoll;
	*pPitch = atan2(2 * (y * z + w * x), w * w - x * x - y * y + z * z) / 3.14159265358979323846 * 180.0;
	*pYaw = asin(2 * (w * y - x * z)) / 3.14159265358979323846 * 180.0;
	*pRoll = atan2(2 * (x * y + w * z), w * w + x * x - y * y - z * z) / 3.14159265358979323846 * 180.0;

	// clamp rotation values in degrees to a specified range of values to control the refresh rate
	//double increment = 5.0f;
	//*pPitch = static_cast<int>((dPitch + increment / 2.0 * (dPitch > 0 ? 1.0 : -1.0)) / increment) * static_cast<int>(increment);
	//*pYaw = static_cast<int>((dYaw + increment / 2.0 * (dYaw > 0 ? 1.0 : -1.0)) / increment) * static_cast<int>(increment);
	//*pRoll = static_cast<int>((dRoll + increment / 2.0 * (dRoll > 0 ? 1.0 : -1.0)) / increment) * static_cast<int>(increment);
}

Point2f CK4Wv2OpenCVModule::BodyToScreen(const CameraSpacePoint& bodyPoint)
{
	// Calculate the body's position on the screen
	ColorSpacePoint colorPoint = { 0 };
	m_pCoordinateMapper->MapCameraPointToColorSpace(bodyPoint, &colorPoint);

	float screenPointX = static_cast<float>(colorPoint.X);
	float screenPointY = static_cast<float>(colorPoint.Y);

	return Point2f(screenPointX, screenPointY);
}
