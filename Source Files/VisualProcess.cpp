#include <string>
#include "GLFrame.h"
#include "model.h"
#include "VisualProcess.h"

using namespace std;
using namespace cv;


bool AcquireFrame(Mat &frame, FRAME_TYPE ftype){
	if(ftype==IMAGE){
		string path;
		//cin>>path;
		path = "C:\\Users\\Administrator\\Desktop\\myFace100\\Resource Files\\face_woman.png";
		
		frame = imread(path);
		if(frame.empty()){
			cerr<<"Invalid image path!"<<endl;
			return false;
		}else{
			cout<<"open file "<<path<<"succeeded.\n";
			return true;
		}
	}else if(ftype==CAMERA){
		VideoCapture vcap(0);
		if(!vcap.isOpened()){
			cerr<<"Camera didn't work!"<<endl;
			return false;
		}

		vcap>>frame;
		if(frame.empty()){
			cerr<<"Failed to capture a frame via camera!"<<endl;
			return false;
		}
		return true;
	}else{
		return false;
	}
}



void HeadPoseEstimation(const M3DVector3f *landmarks, const Model& model, GLFrame* obj_frame, M3DMatrix44f &scaler, const M3DMatrix44f p){
	float roll_angle = 0, s = 1;
	float scaler_height = 1;  //高度缩放因子
	float translation[2];
	M3DVector3f facial_feature_points[3], temp;
	M3DMatrix44f m;

	obj_frame->SetOrigin(0.0f,0.0f,0.0f);
	obj_frame->SetForwardVector(0.0f,0.0f,-1.0f);
	obj_frame->SetUpVector(0.0f,1.0f,0.0f);


	/*roll_angle = atan((landmarks[LM_OUTER_CORNER_OF_LEFT_EYE][1]-landmarks[LM_OUTER_CORNER_OF_RIGHT_EYE][1])/(landmarks[LM_OUTER_CORNER_OF_LEFT_EYE][0]-landmarks[LM_OUTER_CORNER_OF_RIGHT_EYE][0]));
	roll_angle += atan((landmarks[LM_INNER_CORNER_OF_LEFT_EYE][1]-landmarks[LM_INNER_CORNER_OF_RIGHT_EYE][1])/(landmarks[LM_INNER_CORNER_OF_LEFT_EYE][0]-landmarks[LM_INNER_CORNER_OF_RIGHT_EYE][0]));
	roll_angle += atan((landmarks[LM_LEFT_CORNER_OF_OUTER_LIP_CONTOUR][1]-landmarks[LM_RIGHT_CORNER_OF_OUTER_LIP_CONTOUR][1])/(landmarks[LM_LEFT_CORNER_OF_OUTER_LIP_CONTOUR][0]-landmarks[LM_RIGHT_CORNER_OF_OUTER_LIP_CONTOUR][0]));
	roll_angle /= 3;
	obj_frame->RotateWorld(roll_angle, 0.0f, 0.0f, 1.0f);*/
	//Scaling.
	model.getVertex(UPPER_CONTACT_POINT_BETWEEN_LEFT_EAR_AND_FACE, facial_feature_points[0]);
	model.getVertex(UPPER_CONTACT_POINT_BETWEEN_RIGHT_EAR_AND_FACE, facial_feature_points[1]);
	s = sqrt(pow(landmarks[LM_UPPER_CONTACT_POINT_BETWEEN_LEFT_EAR_AND_FACE][0]
				-landmarks[LM_UPPER_CONTACT_POINT_BETWEEN_RIGHT_EAR_AND_FACE][0],2)
			+pow(landmarks[LM_UPPER_CONTACT_POINT_BETWEEN_LEFT_EAR_AND_FACE][1]
				-landmarks[LM_UPPER_CONTACT_POINT_BETWEEN_RIGHT_EAR_AND_FACE][1],2))
	   /sqrt(pow(facial_feature_points[0][0]-facial_feature_points[1][0],2)
			+pow(facial_feature_points[0][1]-facial_feature_points[1][1],2));

	model.getVertex(16, facial_feature_points[0]);  //
	model.getVertex(49, facial_feature_points[1]);  //
	model.getVertex(10, facial_feature_points[2]);  //下巴对应顶点
	for(int i = 0; i<3; i++){
		m3dTransformVector3(temp, facial_feature_points[i], p);
		facial_feature_points[i][0] = temp[0];
		facial_feature_points[i][1] = temp[1];
		facial_feature_points[i][2] = temp[2];
	}
	scaler_height = sqrt(pow((landmarks[19][0]+landmarks[24][0])/2 - landmarks[8][0],2) + pow((landmarks[19][1]+landmarks[24][1])/2 - landmarks[8][1],2))
					/sqrt(pow((facial_feature_points[0][0]+facial_feature_points[1][0])/2 - facial_feature_points[2][0],2) + pow((facial_feature_points[0][1]+facial_feature_points[1][1])/2 - facial_feature_points[2][1],2));
	m3dScaleMatrix44(scaler, s, scaler_height, s);
	
	//Translation.
	model.getVertex(721, facial_feature_points[0]);
	m3dInvertMatrix44(m, p);
	m3dTransformVector3(temp, landmarks[29], m);  //landmark点要先乘以投影矩阵的逆矩阵
	translation[0] = temp[0] - facial_feature_points[0][0];
	translation[1] = temp[1] - facial_feature_points[0][1];
	obj_frame->TranslateWorld(translation[0], translation[1], 0.0f);
}