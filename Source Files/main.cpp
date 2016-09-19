// Primitieves.cpp
// OpenGL SuperBible, Chapter 2
// Demonstrates the 7 Geometric Primitives
// Program by Richard S. Wright Jr.

#include "clockwise.h"
#include "meshEditor.h"

#include <GLTools.h>	// OpenGL toolkit
#include <GLMatrixStack.h>
#include <GLFrame.h>
#include <GLFrustum.h>
#include <GLBatch.h>
#include <GLGeometryTransform.h>
#include <iostream>

#include "GLSetting.h"
#include "model.h"
#include "myTools.h"
#include "VisualProcess.h"
#include "RBF.h"
#include "io.h"

#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <gl/glut.h>
#endif

#include "LBF.h"

#include <string>
using std::string;
void myLoadShader();
void myUseShader();

using namespace std;
using namespace cv;

/////////////////////////////////////////////////////////////////////////////////
// An assortment of needed classes
GLShaderManager		shaderManager;
GLMatrixStack		modelViewMatrix;
GLMatrixStack		projectionMatrix;
GLFrame             *objectFrame;
GLFrustum			viewFrustum;

GLBatch             background_batch;       //背景图片
GLBatch				*facesBatch;            //Face Model
GLBatch             landmarks_batch;        //facial feature points

GLGeometryTransform	transformPipeline;

GLuint	ADSLightShader;		// The diffuse light shader
GLint	locAmbient;			// The location of the ambient color
GLint   locDiffuse;			// The location of the diffuse color
GLint   locSpecular;		// The location of the specular color
GLint	locLight;			// The location of the Light in eye coordinates
GLint	locMVP;				// The location of the ModelViewProjection matrix uniform
GLint	locMV;				// The location of the ModelView matrix uniform
GLint	locNM;				// The location of the Normal matrix uniform

GLuint texture[2];  //texture[0] for background, texture[1] for model.
int nVerts;         //人脸模型的顶点数目
int nFaces;         //人脸模型的三角面数目

static GLfloat Black[] = { 0.0f, 0.0f, 0.0f, 1.0f};
GLfloat vWhite[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat vGray[] = { 0.38f, 0.38f, 0.38f, 1.0f };
GLfloat texcoords[3][3];

M3DVector3i *Faces;
M3DVector3f Face[3];
M3DVector3f *vertice_array;      //人脸模型的顶点数组
M3DVector3f *landmarks;
M3DVector3f *vertice_landmarks;
M3DVector3f *vertice_test;
M3DMatrix44f scaler;

Model candide3;

Mat frame;

RBF rbf;

bool  current_frame_update = false;   //当前帧是否更新

//double **landmarks;

// Keep track of effects step
int    nStep = 0;
int    menu = 0;
int    nSP = 0;
int    nAP = 0;
int    switch_background = 1;  //背景图片显示开关，0 stands for on, 1 stands for off
int    switch_landmarks = 1;   //人脸特征点显示开关
int    switch_model = 0;       //人脸模型显示开关
int    mode = 1;               //模型渲染模式，0 stands for GL_LINE, 1 stands for GL_FILL
int    FFP_number;             //number of facial feature points
int    fx, fy, fw, fh;  //人脸的位置数据
int    test = 0;
int*   FFP_index;              //index array of facial feature points
int*   LM_index;               //index array of landmarks

//const int    ffpNum   = 11;   //人脸特征点数目

//float ap = 0;
static float  viewport_width;  //视口宽度（背景图片像素数）
static float  viewport_height; //视口高度（背景图片像素数）
static float  window_width;    //窗口宽度（窗口像素数）
static float  window_height;   //窗口高度（窗口像素数）
float  r;               //人脸旋转偏量
float  tx, ty;          //center of tracked face
static float light_x;   //光源的位置，下同。
static float light_y;
static float light_z;

point  t;               //人脸位移偏量
//point  facial_feature_points[ffpNum];             //人脸特征点

FRAME_TYPE frame_type;

bool findVertex(float x, float y, float vertex[3]){
	float t[3];

	m3dTransformVector3(t, vertex, transformPipeline.GetModelViewMatrix());

	if(pow(t[0]-x,2)+pow(t[1]-y,2)<0.001)return true;
	else return false;
}

///////////////////////////////////////////////////////////////////////////////
// This function does any needed initialization on the rendering context. 
// This is the first opportunity to do any OpenGL related tasks.
void SetupRC()
{
	glPointSize(3.0);

    // White background
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f );

	myLoadShader();
	shaderManager.InitializeStockShaders();
	transformPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	objectFrame = new GLFrame;

	m3dScaleMatrix44(scaler, 1, 1, 1);
	
 	glGenTextures(2,texture);

	AcquireFrame(frame,frame_type);//For debugging. Image file path is fixed.
	LoadTexture(frame,texture[BACKGROUND]);
	current_frame_update = true;

	ReadGlobalParamFromFile(modelPath+"LBF.model");

	viewport_width = frame.cols;
	viewport_height = frame.rows;
	glutReshapeWindow(viewport_width,viewport_height);

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

	//////////////////////////////////////////////////////////////////////
	myReadMesh("C:\\Users\\Administrator\\Desktop\\candide3.off", vertice_array, Faces, nVerts, nFaces);
	//myWriteMesh(Verts,Faces,candide3.nVertex(),candide3.nFace());
	//
	/*nVerts = candide3.nVertex();
	nFaces = candide3.nFace();*/

	candide3.applySP();
	candide3.applyAP();

	/*vertice_array = new M3DVector3f[nVerts];
	for (int i = 0; i<nVerts; i++){
		candide3.getTransCoords(i,vertice_array[i]);
	}

	Faces = new M3DVector3i[nFaces];
	for (int i = 0; i<nFaces; i++)
		candide3.getFace(i,Faces[i]);*/
}


///////////////////////////////////////////////////////////////////////////////
// Called to draw scene
void RenderScene(void){
	M3DVector3f light_position = {0, -1, -10};  //点光源的位置

	// Clear the window with current clearing color
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	//Should not be here./////////////////////////////////////////////////////////////////
	//检测特征点                                                                        //
	if(current_frame_update == true){                                                   //
		FaceDetectionAndAlignment(frame, landmarks);                                    //
		current_frame_update = false;                                                   //
		                                                                                //
		//landmarks的值从像素点位置转换为视口坐标系位置                                 //
		for(int i = 0; i<global_params.landmark_num; i++){                              //
			landmarks[i][0] = (2*landmarks[i][0]-viewport_width)/viewport_width;        //
			landmarks[i][1] = (viewport_height-2*landmarks[i][1])/viewport_height;      //
			landmarks[i][2] = landmarks[i][2];                                          //
		}                                                                               //
	}/////////////////////////////////////////////////////////////////////////////////////
	
	modelViewMatrix.PushMatrix();

		//是否显示背景图，0为显示。
		if(switch_background == 0){
			shaderManager.UseStockShader(GLT_SHADER_TEXTURE_REPLACE, transformPipeline.GetModelViewProjectionMatrix(),0);
			background_batch.Draw();
		} 

		//渲染模型前的矩阵变换。
		M3DMatrix44f mObjectFrame;
		objectFrame->GetMatrix(mObjectFrame);
		modelViewMatrix.MultMatrix(mObjectFrame);
		modelViewMatrix.MultMatrix(scaler);

		//是否渲染模型，0为渲染。
		if(switch_model == 0){
			glBindTexture(GL_TEXTURE_2D,texture[1]);
			delete facesBatch;
			facesBatch = NULL;
			facesBatch = new GLBatch;
			facesBatch->Begin(GL_TRIANGLES, nFaces*3,1);
			for(int i = 0; i < nFaces; i++){
				for(int j = 0; j < 3; j++){
					candide3.getTexCoords(Faces[i][j],texcoords[j][0],texcoords[j][1]);
					facesBatch->MultiTexCoord2f(0,texcoords[j][0],texcoords[j][1]);
					facesBatch->Vertex3f(vertice_array[Faces[i][j]][0],vertice_array[Faces[i][j]][1],vertice_array[Faces[i][j]][2]);
				}
			}
			facesBatch->End();

			//Render in GL_LINE mode or GL_FILL mode.
			if(!mode)
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			else
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

			shaderManager.UseStockShader(GLT_SHADER_TEXTURE_REPLACE, transformPipeline.GetModelViewProjectionMatrix(),0);
			//myUseShader();
			facesBatch->Draw();
		}

		//是否标识出landmarks，0为标识。
		if(switch_landmarks == 0){
			landmarks_batch.Begin(GL_POINTS, global_params.landmark_num-test);
			landmarks_batch.CopyVertexData3f(landmarks);
			landmarks_batch.End();

			shaderManager.UseStockShader(GLT_SHADER_IDENTITY, vWhite);  //不进行modelview变换，直接渲染。
			landmarks_batch.Draw();
		}


	modelViewMatrix.PopMatrix();  //清楚模型的矩阵变换信息。

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindTexture(GL_TEXTURE_2D,texture[BACKGROUND]);
	// Flush drawing commands
	glutSwapBuffers();
   }


// Respond to arrow keys by moving the camera frame of reference
void SpecialKeys(int key, int x, int y){

	if(key == GLUT_KEY_UP)
		objectFrame->TranslateWorld(0.0f, 0.01f, 0.0f);
    
	if(key == GLUT_KEY_DOWN)
		objectFrame->TranslateWorld(0.0f, -0.01f, 0.0f);
	
	if(key == GLUT_KEY_LEFT)
		objectFrame->TranslateWorld(-0.01f, 0.0f, 0.0f);
    
	if(key == GLUT_KEY_RIGHT)
		objectFrame->TranslateWorld(0.01f, 0.0f, 0.0f);
    
	glutPostRedisplay();
}


///////////////////////////////////////////////////////////////////////////////
// A normal ASCII key has been pressed.
// In this case, advance the scene when the space bar is pressed
void KeyPressFunc(unsigned char key, int x, int y){

	if(key == 56)
		objectFrame->RotateWorld(m3dDegToRad(9.0f), 1.0f, 0.0f, 0.0f);
    
	if(key == 53)
		objectFrame->RotateWorld(m3dDegToRad(-9.0f), 1.0f, 0.0f, 0.0f);
	
	if(key == 52)
		objectFrame->RotateWorld(m3dDegToRad(9.0f), 0.0f, 1.0f, 0.0f);
    
	if(key == 54)
		objectFrame->RotateWorld(m3dDegToRad(-9.0f), 0.0f, 1.0f, 0.0f);

	if(key == 55)
		objectFrame->RotateWorld(m3dDegToRad(1.5f), 0.0f, 0.0f, 1.0f);

	if(key == 57)
		objectFrame->RotateWorld(m3dDegToRad(-1.5f), 0.0f, 0.0f, 1.0f);

	//Enter键, update texture coordinates.
	if(key == 13){
		nStep = 1;
		modelViewMatrix.PushMatrix();

			M3DMatrix44f mObjectFrame;
			objectFrame->GetMatrix(mObjectFrame);
			modelViewMatrix.MultMatrix(mObjectFrame);
			modelViewMatrix.MultMatrix(scaler);

			GLfloat t[3];
			float x, y;
			if(viewport_width>=viewport_height) x = viewport_width/viewport_height, y = 1;
			else x = 1, y = viewport_height/viewport_width;
			for(int i = 0; i < nFaces; i++){
				for(int j = 0; j < 3; j++){
					t[0] = vertice_array[Faces[i][j]][0];
					t[1] = vertice_array[Faces[i][j]][0];
					t[2] = vertice_array[Faces[i][j]][0];
					m3dTransformVector3(texcoords[j], t, transformPipeline.GetModelViewMatrix());
					/*纹理坐标规范化，坐标值范围为0.0到1.0*/
					candide3.setTexCoords(Faces[i][j],(texcoords[j][0]/x+1)/2,(-texcoords[j][1]/y+1)/2);
				}
			}

		modelViewMatrix.PopMatrix();

		LoadTexture(frame,texture[MODEL]);
	}
	
	//空白键
	if(key == 32){
		mode = ++mode %2;
	}

	//"+"键
	if(key == 43){
		switch(menu){
		case 0:
			M3DMatrix44f s;
			m3dScaleMatrix44(s, 1.05, 1.05, 1.05);
			m3dMatrixMultiply44(scaler,scaler,s);
			break;
		case 1:
			candide3.addSP(nSP,0.1);
			//candide3.applySP(nSP);
			break;
		case 2:
			/*ap += 0.5;
			candide3.setAP(0,ap);
			candide3.applyAP();*/
			candide3.addAP(nAP,0.1);
			//candide3.applyAP(nAP);
			break;
		}
		for (int i = 0; i < nVerts; i++){
			candide3.getTransCoords(i,vertice_array[i]);
		}
	}

	 //"-"键
	if(key == 45){
		switch(menu){
		case 0:
			M3DMatrix44f s;
			m3dScaleMatrix44(s, 0.95, 0.95, 0.95);
			m3dMatrixMultiply44(scaler,scaler,s);
			break;
		case 1:
			candide3.addSP(nSP,-0.1);
			//candide3.applySP(nSP);
			break;
		case 2:
			/*ap -= 0.5;
			candide3.setAP(0,ap);
			candide3.applyAP();*/
			candide3.addAP(nAP,-0.1);
			//candide3.applyAP(nAP);
			break;
		}
		for (int i = 0; i < nVerts; i++){
			candide3.getTransCoords(i,vertice_array[i]);
		}
	}

	//清零,回到标准Candide-3模型
	if(key == 48){
		M3DVector3f Vert;
		for (int i = 0; i<nVerts; i++){
			candide3.getVertex(i,Vert);
			candide3.setTransCoords(i,Vert);
			vertice_array[i][0] = Vert[0], vertice_array[i][1] = Vert[1], vertice_array[i][2] = Vert[2];			
		}
		m3dScaleMatrix44(scaler, 1, 1, 1);
		objectFrame->SetOrigin(0,0,0);
		objectFrame->SetForwardVector(0,0,-1);
		objectFrame->SetUpVector(0,1,0);
		//ap = 0;
		for(int i = 0; i < candide3.nSUs(); i++)
			candide3.setSP(i,0);
		//candide3.setAP(0,ap);candide3.applyAP();
		for(int i = 0; i < candide3.nAUs(); i++)
			candide3.setAP(i,0);
	}

	//回到初始位置和静态样貌
	if(key == 49){
		m3dScaleMatrix44(scaler, 1, 1, 1);
		delete objectFrame; objectFrame = NULL;
		objectFrame = new GLFrame;

		candide3.clearAP();
		M3DVector3f Vert;
		for (int i = 0; i<nVerts; i++){
			candide3.setTransCoords(i,Vert);
			vertice_array[i][0] = Vert[0], vertice_array[i][1] = Vert[1], vertice_array[i][2] = Vert[2];
		}

	}

	if(key == ('c'&0x1f)){
		int mod = glutGetModifiers();
		if(mod == GLUT_ACTIVE_CTRL){
			LONG len = frame.cols * frame.rows * 3;
			cout<<"ctrl+c"<<endl;
			HGLOBAL hClipData = GlobalAlloc(GHND, len*sizeof(uchar));
			BYTE *pClipData = (BYTE *)GlobalLock(hClipData);
			memcpy(pClipData, frame.data, len);
			GlobalUnlock(hClipData);			

			if(OpenClipboard(NULL)){
				cout<<"Clipboard on!"<<endl;
				EmptyClipboard();
				SetClipboardData(CF_BITMAP, hClipData);
				CloseClipboard();
			}
		}
	}

	//移动光源位置。
	if(key == 'd'){
		light_x += 0.1;
	}
	if(key == 'a'){
		light_x -= 0.1;
	}
	if(key == 'w'){
		light_y += 0.1;
	}
	if(key == 's'){
		light_y -= 0.1;
	}
	if(key == 'q'){
		light_z += 0.1;
	}
	if(key == 'e'){
		light_z -= 0.1;
	}
                
    glutPostRedisplay();
}

///////////////////////////////////////////////////////////////////////////////
void ProcessMainMenu(int value){
	switch (value){
	case 0:
		menu = value;
		break;
	case 1:
		candide3.loadTexImage("teximage");
		break;	
	case 2:
		break;
	case 3:
		HeadPoseEstimation(landmarks, candide3, objectFrame, scaler);
		cout<<"testing"<<endl;
		glutPostRedisplay();
		break;
	default:;
	}
}

void ProcessSUMenu(int value){
	menu = 1, nSP = value;
}

void ProcessAUMenu(int value){
	menu = 2, nAP = value;
}

void ProcessRenderMenu(int value){
	switch(value){
		case 0:
			switch_background = ++switch_background % 2;
			break;
		case 1:
			switch_landmarks = ++switch_landmarks % 2;
			//test++;
			break;
		case 2:
			switch_model = ++switch_model % 2;
			break;
		default:;
	}
	glutPostRedisplay();
}

void ProcessKernelFuncMenu(int value){
	switch(value){
		case 0: rbf.set_kernel_type(GAUSS);break;
		case 1: rbf.set_kernel_type(REFLECTED_SIGMOIDAL);break;
		case 2: rbf.set_kernel_type(INVERSE_MULTIQUADRICS);break;
		default: cerr<<"Error in choosing kernel function!"<<endl;
	}
}

void ProcessMeshDeformationMenu(int value){
	switch(value){
		case 0:
			double s;
			cout<<"Please input sigma: ";
			cin>>s;
			rbf.set_sigma(s);
			break;
		case 1:{
			ReadFileFFP("C:\\Users\\Administrator\\Desktop\\myFace100\\myFace100\\Resource Files\\facial feature points.txt", FFP_number, FFP_index, LM_index);

			/*Mat FFP_array(2, FFP_number, CV_32F);
			Mat LM_array(2, FFP_number, CV_32F);
			Mat mesh_train(2, candide3.nVertex(), CV_32F);
			Mat mesh_predict(2, candide3.nVertex(), CV_32F);*/
			Mat FFP_array(FFP_number, 2, CV_32F);
			Mat LM_array(FFP_number, 2, CV_32F);
			Mat mesh_train(candide3.nVertex(), 2, CV_32F);
			Mat mesh_predict(candide3.nVertex(), 2, CV_32F);

			modelViewMatrix.PushMatrix();

				M3DVector3f v,temp,t;
				M3DMatrix44f mObjectFrame, imvp;
				objectFrame->GetMatrix(mObjectFrame);
				modelViewMatrix.MultMatrix(mObjectFrame);
				modelViewMatrix.MultMatrix(scaler);
				m3dInvertMatrix44(imvp, transformPipeline.GetModelViewProjectionMatrix());
				for(int i = 0;i<FFP_number;i++){
					/*FFP_array.at<float>(0,i) = Verts[FFP_index[i]*3+0];
					FFP_array.at<float>(1,i) = Verts[FFP_index[i]*3+1];*/
					/*temp[0] = Verts[FFP_index[i]*3+0];
					temp[1] = Verts[FFP_index[i]*3+1];
					temp[2] = Verts[FFP_index[i]*3+2];
					m3dTransformVector3(t, temp, transformPipeline.GetModelViewMatrix());
					FFP_array.at<float>(i,0) = t[0];
					FFP_array.at<float>(i,1) = t[1];*/
					///////////////////////////////////////////////////////
					FFP_array.at<float>(i,0) = vertice_array[FFP_index[i]][0]; //
					FFP_array.at<float>(i,1) = vertice_array[FFP_index[i]][1]; //
					                                                    //
					m3dTransformVector3(v, landmarks[LM_index[i]], imvp);//
					LM_array.at<float>(i,0) = v[0];                      //
					LM_array.at<float>(i,1) = v[1];                      //
					/*LM_array.at<float>(i,0) = landmarks[LM_index[i]][0];
					LM_array.at<float>(i,1) = landmarks[LM_index[i]][1];*/
				}

			//modelViewMatrix.PopMatrix();

			for(int i = 0;i<candide3.nVertex();i++){
				/*mesh_train.at<float>(0,i) = Verts[i*3+0];
				mesh_train.at<float>(1,i) = Verts[i*3+1];*/
				/*temp[0] = Verts[i*3+0];
				temp[1] = Verts[i*3+1];
				temp[2] = Verts[i*3+2];
				m3dTransformVector3(t, temp, transformPipeline.GetModelViewMatrix());
				mesh_train.at<float>(i,0) = t[0];
				mesh_train.at<float>(i,1) = t[1];*/
				//
				mesh_train.at<float>(i,0) = vertice_array[i][0];
				mesh_train.at<float>(i,1) = vertice_array[i][1];
				/*For testing.
				mesh_train.at<float>(2,i) = Verts[i*3+2];*/
			}

			rbf.train(FFP_array, LM_array);
			rbf.predict(mesh_train, mesh_predict);

			////////////////////////////////////////////////////////////////////////
			/*CvANN_MLP_TrainParams params;
			CvANN_MLP model;
			Mat layerSizes = (Mat_<int>(1,3)<<2,FFP_number,2);
			model.create(layerSizes,CvANN_MLP::GAUSSIAN, 0.4, 1.1);
			model.train(FFP_array, LM_array, Mat(), Mat(), params);
			model.predict(mesh_train, mesh_predict);*/
			///////////////////////////////////////////////////////////////////////

			cout<<endl;
			for(int i = 0;i<candide3.nVertex();i++){
				/*Verts[i*3+0] = mesh_predict.at<float>(0,i);
				Verts[i*3+1] = mesh_predict.at<float>(1,i);*/
				vertice_array[i][0] = mesh_predict.at<float>(i,0);
				vertice_array[i][1] = mesh_predict.at<float>(i,1);

				//cout<<Verts[FFP_index[i]*3+0]<<' '<<Verts[FFP_index[i]*3+1]<<endl;
			}

			for(int i = 0;i<FFP_number;i++){
				vertice_array[FFP_index[i]][0] = LM_array.at<float>(i,0);
				vertice_array[FFP_index[i]][1] = LM_array.at<float>(i,1);
				//
				/*Verts[FFP_index[i]*3+0] = landmarks[LM_index[i]][0];
				Verts[FFP_index[i]*3+1] = landmarks[LM_index[i]][1];*/
			}

			/*for(int i = 0;i<candide3.nVertex();i++){
				temp[0] = Verts[i*3+0];
				temp[1] = Verts[i*3+1];
				temp[2] = Verts[i*3+2];
				m3dTransformVector3(t, temp, imvp);
				Verts[i*3+0] = t[0];
				Verts[i*3+1] = t[1];
				Verts[i*3+2] = t[2];
			}*/

			modelViewMatrix.PopMatrix();//
			glutPostRedisplay();
			break;
		}
		default:
			cerr<<"Error in mesh deformation!"<<endl;
	}
}

void processMouse(int button, int state, int x, int y){
	float xx =x, yy = y, mouse[2];
	if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN){
		if(viewport_width>=viewport_height){
			mouse[0] = (2*xx - window_width)/window_height;
			mouse[1] = 1 - 2*yy/window_height;
			cout<<mouse[0]<<','<<mouse[1]<<endl;
		}else{
			mouse[0] = 2*xx/window_width - 1;
			mouse[1] = (window_height-2*yy)/window_width;
			cout<<mouse[0]<<','<<mouse[1]<<endl;
		}

		float t[3];
		for(int i = 0; i < nVerts; i++){
			t[0] = vertice_array[i][0];
			t[1] = vertice_array[i][1];
			t[2] = vertice_array[i][2];
			if(findVertex(mouse[0],mouse[1],t))cout<<i<<endl;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// Window has changed size, or has just been created. In either case, we need
// to use the window dimensions to set the viewport and the projection matrix.
void ChangeSize(int w, int h)
{
	window_width = w;
	window_height = h;

	//将视口缩放并居中于窗口区域
	float ratio = viewport_width/viewport_height;
	float width_scaler = window_width/viewport_width; 
	float height_scaler = window_height/viewport_height;

	float scaler = width_scaler<height_scaler? width_scaler : height_scaler;
    float x = (window_width - viewport_width*scaler)/2;
	float y = (window_height - viewport_height*scaler)/2;

	glViewport(x, y, viewport_width*scaler, viewport_height*scaler);

	//视口坐标为正投影坐标
	//x: -ratio~ratio (-1~1)
	//y:-1~1 (-1/ratio~1/ratio)
	//z:-1~1
	if(ratio>1)
		viewFrustum.SetOrthographic(-ratio,ratio,-1,1,-1.0f,1.0f);
	else
		viewFrustum.SetOrthographic(-1,1,-1/ratio,1/ratio,-1.0f,1.0f);
	
	projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
	modelViewMatrix.LoadIdentity();
}

void TimeFunc(int value){
	RenderScene();
	glutTimerFunc(40,TimeFunc,0);
}

///////////////////////////////////////////////////////////////////////////////
// Main entry point for GLUT based programs
int main(int argc, char* argv[]){

	string frame_type_string;

	cout<<"Please choose [load an image/capture from camera]:"<<endl;
	cout<<"Input \"image\" for loading an image"<<endl;
	cout<<"Input \"camera\" for capturing from camera"<<endl;
	cout<<'>';
	//cin>>frame_type_string;
	//For debugging.	
	cout<<"image"<<endl;
	frame_type_string = "image";
	//For debugging.

	if(frame_type_string=="image"){
		frame_type = IMAGE;
	}else if(frame_type_string=="camera"){
		frame_type = CAMERA;
		glutTimerFunc(40,TimeFunc,0);
	}else{
		cerr<<"invalid input!"<<endl;
		return -1;
	}

	gltSetWorkingDirectory(argv[0]);
	
	glutInit(&argc, argv);
	//glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(500, 500);
	glutCreateWindow("AutoFace 2.0");
    glutReshapeFunc(ChangeSize);
    glutKeyboardFunc(KeyPressFunc);
    glutSpecialFunc(SpecialKeys);
    glutDisplayFunc(RenderScene);

	candide3.open("C:\\Users\\Administrator\\Desktop\\myFace100\\myFace100\\Resource Files\\candide3_all_triangles_ccw.wfm");

	////////////////////////////////////////////////////////
	//创建菜单
	int SUMenu = glutCreateMenu(ProcessSUMenu);
	for(int i = 0; i < candide3.nSUs(); i++){
		glutAddMenuEntry(candide3.getSUsName(i),i);
	}

	int AUMenu = glutCreateMenu(ProcessAUMenu);
	for(int i = 0; i < candide3.nAUs(); i++){
		glutAddMenuEntry(candide3.getAUsName(i),i);
	}

	//Rendering Menu: change rendering mode.
	int RenderMenu = glutCreateMenu(ProcessRenderMenu);
	glutAddMenuEntry("background(on/off)",0);
	glutAddMenuEntry("landmarks(on/off)",1);
	glutAddMenuEntry("model(on/off)",2);

	int KernelFuncMenu = glutCreateMenu(ProcessKernelFuncMenu);
	glutAddMenuEntry("GAUSS",0);
	glutAddMenuEntry("REFLECTED_SIGMOIDAL",1);
	glutAddMenuEntry("INVERSE_MULTIQUADRICS",2);

	int MeshDeformationMenu = glutCreateMenu(ProcessMeshDeformationMenu);
	glutAddSubMenu("Kernel Function",KernelFuncMenu);
	glutAddMenuEntry("set sigma",0);
	glutAddMenuEntry("RBF Transform",1);

	glutCreateMenu(ProcessMainMenu);
    glutAddMenuEntry("Zoom",0);
    glutAddSubMenu("Adjust SU",SUMenu);
	glutAddSubMenu("Change AU",AUMenu);
	glutAddSubMenu("render",RenderMenu);
	glutAddMenuEntry("Load Texture Image",1);
	glutAddMenuEntry("Select FFPs",2);
	glutAddMenuEntry("Head Pose Estimation",3);
	glutAddSubMenu("Mesh Deformation",MeshDeformationMenu);

    glutAttachMenu(GLUT_RIGHT_BUTTON);
	////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////   
	//鼠标操作
	glutMouseFunc(processMouse);
	///////////////////////////////////////////////////////////

	GLenum err = glewInit();
	if (GLEW_OK != err) {
		fprintf(stderr, "GLEW Error: %s\n", glewGetErrorString(err));
		return 1;
	}

	SetupRC();

	glutMainLoop();

	return 0;
}

void myLoadShader(){
	string path = "D:\\BaiduYunDownload\\OpenGLSB5\\SB5\\Src\\Chapter06\\ADSPhong\\";
	ADSLightShader = shaderManager.LoadShaderPairWithAttributes((path+"ADSPhong.vp").c_str(), (path+"ADSPhong.fp").c_str(), 2, GLT_ATTRIBUTE_VERTEX, "vVertex",
			GLT_ATTRIBUTE_NORMAL, "vNormal");

	locAmbient = glGetUniformLocation(ADSLightShader, "ambientColor");
	locDiffuse = glGetUniformLocation(ADSLightShader, "diffuseColor");
	locSpecular = glGetUniformLocation(ADSLightShader, "specularColor");
	locLight = glGetUniformLocation(ADSLightShader, "vLightPosition");
	locMVP = glGetUniformLocation(ADSLightShader, "mvpMatrix");
	locMV  = glGetUniformLocation(ADSLightShader, "mvMatrix");
	locNM  = glGetUniformLocation(ADSLightShader, "normalMatrix");
}

void myUseShader(){
	GLfloat vEyeLight[] = { light_x, light_y, light_z };
	GLfloat vAmbientColor[] = { 0.1f, 0.1f, 0.1f, 1.0f };
	GLfloat vDiffuseColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat vSpecularColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };

	glUseProgram(ADSLightShader);
	glUniform4fv(locAmbient, 1, vAmbientColor);
	glUniform4fv(locDiffuse, 1, vDiffuseColor);
	glUniform4fv(locSpecular, 1, vSpecularColor);
	glUniform3fv(locLight, 1, vEyeLight);
	glUniformMatrix4fv(locMVP, 1, GL_FALSE, transformPipeline.GetModelViewProjectionMatrix());
	glUniformMatrix4fv(locMV, 1, GL_FALSE, transformPipeline.GetModelViewMatrix());
	glUniformMatrix3fv(locNM, 1, GL_FALSE, transformPipeline.GetNormalMatrix());
}