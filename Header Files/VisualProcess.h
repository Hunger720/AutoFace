#include"macros.h"

#ifndef __OPENCV_HPP__
#define __OPENCV_HPP__
#include <opencv2/opencv.hpp>
#endif


///<summary>
///Get a frame from a video device or an image.
///</summary>
///<param name="frame">The frame.</param>
///<param name="ftype">The input type of the frame.</param>
///<returns>status</returns>
bool AcquireFrame(cv::Mat &frame, FRAME_TYPE ftype);

///<summary>
///Get pose information of the human face in a frame.
///</summary>
///<param name="translation">Parameter type is float[3], represents (x,y,z).</param>
///<param name="scaling">Parameter type is float[3], represents (Sx,Sy,Sz).</param>
///<returns>status</returns>
void HeadPoseEstimation(const M3DVector3f *landmarks, const Model& model, GLFrame* obj_frame, M3DMatrix44f &scaler, const M3DMatrix44f p);