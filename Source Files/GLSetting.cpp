#include <iostream>
#include <GLTools.h>
#include "GLSetting.h"

using namespace std;

bool LoadTexture(cv::Mat frame, GLuint &texObj){
	if(frame.empty()){
		cerr<<"empty frame!"<<endl;
		return false;
	}

	glBindTexture(GL_TEXTURE_2D,texObj);

	if(!glIsTexture(texObj)){
		cerr<<"invalid texture object!"<<endl;
		return false;
	}

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frame.cols, frame.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, frame.data);
	glGenerateMipmap(GL_TEXTURE_2D);

	return true;
}

void loadBackground(float viewport_width, float viewport_height, GLBatch& background_batch){
	float x, y;
	if(viewport_width >= viewport_height)
		x = viewport_width/viewport_height, y = 1;
	else
		x = 1, y = viewport_height/viewport_width;

	//Background Texture
	background_batch.Begin(GL_TRIANGLES,6,1);
	background_batch.MultiTexCoord2f(0,0.0f,0.0f);
    background_batch.Vertex3f(-x,y,-.5f);
	background_batch.MultiTexCoord2f(0,0.0f,1.0f);
    background_batch.Vertex3f(-x,-y,-.5f);
	background_batch.MultiTexCoord2f(0,1.0f,0.0f);
    background_batch.Vertex3f(x,y,-.5f);
	
	background_batch.MultiTexCoord2f(0,0.0f,1.0f);
    background_batch.Vertex3f(-x,-y,-.5f);
	background_batch.MultiTexCoord2f(0,1.0f,1.0f);
    background_batch.Vertex3f(x,-y,-.5f);
	background_batch.MultiTexCoord2f(0,1.0f,0.0f);
    background_batch.Vertex3f(x,y,-.5f);
	background_batch.End();
}


//GLUT Menu Functions.
void open(Model& model){
	const char* path = "C:\\Users\\Administrator\\Desktop\\myFace100\\myFace100\\Resource Files\\human_face.wfm";
	cout<<"Path: "<<path<<endl;
	if(model.open(path)) cout<<"Open wfm file succeeded."<<endl;
	else cerr<<"Open wfm file failed!"<<endl;
}

void save(Model model){
	string path;
	cout<<"Path: ";
	cin>>path;
	if(model.write(path.c_str())) cout<<"Save wfm file succeeded."<<endl;
	else cerr<<"Save wfm file failed!"<<endl;
}

void GLControl(){
}