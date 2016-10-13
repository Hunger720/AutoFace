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
int nVerts = 0;     //人脸模型的顶点数目
int nFaces = 0;     //人脸模型的三角面数目

static GLfloat Black[] = { 0.0f, 0.0f, 0.0f, 1.0f};
GLfloat vWhite[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat texcoords[3][3];

M3DVector3i *Faces;
M3DVector3f Face[3];
M3DVector3f *vertices_array;     //人脸模型的顶点数组
M3DVector3f *pixels_landmarks;   //landmrak的坐标，按像素点计算
M3DVector3f *landmarks;
M3DVector3f *vertice_landmarks;
M3DVector3f *vertice_test;

M3DMatrix44f scaling;            //Scaling matrix of human face mesh.

Model candide3;

Mat frame;

RBF rbf;

bool  current_frame_update = false;   //当前帧是否更新

// Keep track of effects step
int    nStep = 0;
int    menu = 0;               //+、-按键对应的操作
int    nSP = 0;
int    nAP = 0;
int    switch_background = 0;  //背景图片显示开关，0 stands for on, 1 stands for off
int    switch_landmarks = 1;   //人脸特征点显示开关
int    switch_model = 0;       //人脸模型显示开关
int    mode = 0;               //模型渲染模式，0 stands for GL_LINE, 1 stands for GL_FILL
int    FFP_number;             //number of facial feature points
int    fx, fy, fw, fh;  //人脸的位置数据
int    test = 0;
int*   FFP_index;              //index array of facial feature points
int*   LM_index;               //index array of landmarks

static float  viewport_aspect_ratio;  //视口宽高比（背景图片宽高比）
static float  viewport_scaler;        //缩放比例，将背景图片缩放到视口大小
static float  viewport_width = 200;   //视口宽度（背景图片像素数）
static float  viewport_height = 200;  //视口高度（背景图片像素数）
static float  window_width;           //窗口宽度（窗口像素数）
static float  window_height;          //窗口高度（窗口像素数）
float  r;               //人脸旋转偏量
float  tx, ty;          //center of tracked face
static float light_x;   //光源的位置，下同。
static float light_y;
static float light_z;

FRAME_TYPE frame_type;


///////////////////////////////////////////////////////////////////////////////
// This function does any needed initialization on the rendering context. 
// This is the first opportunity to do any OpenGL related tasks.
void SetupRC()
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f );//White background
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glGenTextures(2,texture);
	glPointSize(3.0);

	//myLoadShader();
	shaderManager.InitializeStockShaders();
	transformPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);

	objectFrame = new GLFrame;
	m3dScaleMatrix44(scaling,1,1,1);

}


///////////////////////////////////////////////////////////////////////////////
// Called to draw scene
void RenderScene(void){
	M3DVector3f light_position = {0, -1, -10};  //点光源的位置

	// Clear the window with current clearing color
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	//是否显示背景图，0为显示。
	if(switch_background == 0){
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		shaderManager.UseStockShader(GLT_SHADER_TEXTURE_REPLACE, transformPipeline.GetProjectionMatrix(),0);//只乘以投影矩阵
		background_batch.Draw();
	}
	
	modelViewMatrix.PushMatrix();
		//渲染模型前的矩阵变换。
		M3DMatrix44f mObjectFrame;
		objectFrame->GetMatrix(mObjectFrame);
		modelViewMatrix.MultMatrix(mObjectFrame);
		modelViewMatrix.MultMatrix(scaling);
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
					facesBatch->Vertex3f(vertices_array[Faces[i][j]][0],vertices_array[Faces[i][j]][1],vertices_array[Faces[i][j]][2]);
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
	modelViewMatrix.PopMatrix();//清除模型的矩阵变换信息。 

	//是否标识出landmarks，0为标识。
	if(switch_landmarks == 0){
		shaderManager.UseStockShader(GLT_SHADER_IDENTITY, vWhite);//不进行modelview和projection变换，直接渲染
		landmarks_batch.Begin(GL_POINTS, global_params.landmark_num);
		landmarks_batch.CopyVertexData3f(landmarks);
		landmarks_batch.End();
		landmarks_batch.Draw();
	}

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

	if(key == '8')
		objectFrame->RotateWorld(m3dDegToRad(9.0f), 1.0f, 0.0f, 0.0f);    
	if(key == '5')
		objectFrame->RotateWorld(m3dDegToRad(-9.0f), 1.0f, 0.0f, 0.0f);
	if(key == '4')
		objectFrame->RotateWorld(m3dDegToRad(9.0f), 0.0f, 1.0f, 0.0f);
	if(key == '6')
		objectFrame->RotateWorld(m3dDegToRad(-9.0f), 0.0f, 1.0f, 0.0f);
	if(key == '7')
		objectFrame->RotateWorld(m3dDegToRad(1.5f), 0.0f, 0.0f, 1.0f);
	if(key == '9')
		objectFrame->RotateWorld(m3dDegToRad(-1.5f), 0.0f, 0.0f, 1.0f);

	//Enter键, update texture coordinates.
	if(key == 13){
		nStep = 1;
		modelViewMatrix.PushMatrix();

			M3DMatrix44f mObjectFrame;
			objectFrame->GetMatrix(mObjectFrame);
			modelViewMatrix.MultMatrix(mObjectFrame);

			GLfloat t[3];
			float x, y;
			if(viewport_width>=viewport_height) x = viewport_width/viewport_height, y = 1;
			else x = 1, y = viewport_height/viewport_width;
			for(int i = 0; i < nFaces; i++){
				for(int j = 0; j < 3; j++){
					t[0] = vertices_array[Faces[i][j]][0];
					t[1] = vertices_array[Faces[i][j]][1];
					t[2] = vertices_array[Faces[i][j]][2];
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
			modelViewMatrix.Scale(1.05,1.05,1.05);
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
			candide3.getTransCoords(i,vertices_array[i]);
		}
	}

	 //"-"键
	if(key == 45){
		switch(menu){
		case 0:
			modelViewMatrix.Scale(0.95,0.95,0.95);
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
			candide3.getTransCoords(i,vertices_array[i]);
		}
	}

	//按键0,清零,回到标准Candide-3模型
	if(key == '0'){
		M3DVector3f Vert;
		for (int i = 0; i<nVerts; i++){
			candide3.getVertex(i,Vert);
			candide3.setTransCoords(i,Vert);
			vertices_array[i][0] = Vert[0], vertices_array[i][1] = Vert[1], vertices_array[i][2] = Vert[2];			
		}
		
		objectFrame->SetOrigin(0,0,0);
		objectFrame->SetForwardVector(0,0,-1);
		objectFrame->SetUpVector(0,1,0);
		
		for(int i = 0; i < candide3.nSUs(); i++)
			candide3.setSP(i,0);
		for(int i = 0; i < candide3.nAUs(); i++)
			candide3.setAP(i,0);
		while(modelViewMatrix.GetLastError()!=GLT_STACK_UNDERFLOW)
			modelViewMatrix.PopMatrix();
		modelViewMatrix.LoadIdentity();
	}

	//按键1,回到初始位置和静态样貌
	if(key == '1'){
		
		objectFrame->SetOrigin(0,0,0);
		objectFrame->SetForwardVector(0,0,-1);
		objectFrame->SetUpVector(0,1,0);

		candide3.clearAP();
		M3DVector3f Vert;
		for (int i = 0; i<nVerts; i++){
			candide3.getTransCoords(i,Vert);
			vertices_array[i][0] = Vert[0], vertices_array[i][1] = Vert[1], vertices_array[i][2] = Vert[2];
		}
		while(modelViewMatrix.GetLastError()!=GLT_STACK_UNDERFLOW)
			modelViewMatrix.PopMatrix();
		modelViewMatrix.LoadIdentity();

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
	if(key == 'd')
		light_x += 0.1;
	if(key == 'a')
		light_x -= 0.1;
	if(key == 'w')
		light_y += 0.1;
	if(key == 's')
		light_y -= 0.1;
	if(key == 'q')
		light_z += 0.1;
	if(key == 'e')
		light_z -= 0.1;
                
    glutPostRedisplay();
}

///////////////////////////////////////////////////////////////////////////////
void ProcessMainMenu(int value){
	M3DVector3f *v = new M3DVector3f[nVerts];
	switch (value){
	case 0:
		menu = value;
		break;
	case 1:  //从图片文件获取帧
		AcquireFrame(frame,IMAGE);
		LoadTexture(frame,texture[BACKGROUND]);
		viewport_width = frame.cols;
		viewport_height = frame.rows;
		glutReshapeWindow(viewport_width,viewport_height);
		loadBackground(viewport_width,viewport_height,background_batch);
		break;
	case 2:  //从摄像头获取帧
		AcquireFrame(frame,CAMERA);
		LoadTexture(frame,texture[BACKGROUND]);
		viewport_width = frame.cols;
		viewport_height = frame.rows;
		glutReshapeWindow(viewport_width,viewport_height);
		loadBackground(viewport_width,viewport_height,background_batch);
		//current_frame_update = true;
		break;
	case 3:  //检测人脸特征点
		ReadGlobalParamFromFile(modelPath+"LBF.model");
		FaceDetectionAndAlignment(frame,pixels_landmarks);
		//landmarks的值从初始窗口像素点位置转换为视口坐标系位置（窗口像素点坐标与视口像素点坐标一致）
		if(landmarks != NULL) delete[] landmarks;
		landmarks = new M3DVector3f[global_params.landmark_num];
		for(int i = 0; i<global_params.landmark_num; i++){
			landmarks[i][0] = (2*pixels_landmarks[i][0]-viewport_width)/viewport_width;
			landmarks[i][1] = (viewport_height-2*pixels_landmarks[i][1])/viewport_height;
			landmarks[i][2] = pixels_landmarks[i][2];
		}
		glutPostRedisplay();
		switch_landmarks = 0;
		break;
	case 4:
		open(candide3);
		//读取顶点数据
		nVerts = candide3.nVertex();
		vertices_array = new M3DVector3f[nVerts];
		for (int i = 0; i<nVerts; i++){
			candide3.getTransCoords(i,vertices_array[i]);
		}
		//读取三角面片数据
		nFaces =candide3.nFace(); 
		Faces = new M3DVector3i[nFaces];
		for (int i = 0; i<nFaces; i++)
			candide3.getFace(i,Faces[i]);
		//读取形变单元数据
		candide3.applySP();
		candide3.applyAP();
		//重新渲染
		glutPostRedisplay();
		break;	
	case 5:
		save(candide3);
		break;
	case 6:  //保存.off文件
		float temp[3];
		modelViewMatrix.PushMatrix();//
		M3DMatrix44f mObjectFrame;
		objectFrame->GetMatrix(mObjectFrame);
		modelViewMatrix.MultMatrix(mObjectFrame);
		modelViewMatrix.MultMatrix(scaling);
		for(int i=0; i<nVerts; i++){
			m3dTransformVector3(temp, vertices_array[i], transformPipeline.GetModelViewProjectionMatrix());
			v[i][0] = temp[0];
			v[i][1] = temp[1];
			v[i][2] = temp[2];
		}
		modelViewMatrix.PopMatrix();//
		myWriteMesh(v,Faces,nVerts,nFaces);
		break;
	case 7:  //Get the head pose information.
		HeadPoseEstimation2(landmarks, candide3, objectFrame, scaling,transformPipeline.GetModelViewProjectionMatrix());
		while(modelViewMatrix.GetLastError()!=GLT_STACK_UNDERFLOW)
			modelViewMatrix.PopMatrix();
		modelViewMatrix.LoadIdentity();
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

			Mat FFP_array(FFP_number, 2, CV_32F);
			Mat LM_array(FFP_number, 2, CV_32F);
			Mat mesh_train(candide3.nVertex(), 2, CV_32F);
			Mat mesh_predict(candide3.nVertex(), 2, CV_32F);

			modelViewMatrix.PushMatrix();
				M3DVector3f v;
				M3DMatrix44f mObjectFrame, imvp;
				objectFrame->GetMatrix(mObjectFrame);
				modelViewMatrix.MultMatrix(mObjectFrame);
				modelViewMatrix.MultMatrix(scaling);
				m3dInvertMatrix44(imvp, transformPipeline.GetModelViewProjectionMatrix());
				//输入训练数据
				for(int i = 0;i<FFP_number;i++){
					//特征点训练数据
					FFP_array.at<float>(i,0) = vertices_array[FFP_index[i]][0];
					FFP_array.at<float>(i,1) = vertices_array[FFP_index[i]][1];
					//先将landmark乘以modelview矩阵的逆矩阵，再输入landmark训练数据
					m3dTransformVector3(v, landmarks[LM_index[i]], imvp);
					LM_array.at<float>(i,0) = v[0];
					LM_array.at<float>(i,1) = v[1];
				}
			modelViewMatrix.PopMatrix();
			//输入待预测数据
			for(int i = 0;i<candide3.nVertex();i++){
				mesh_train.at<float>(i,0) = vertices_array[i][0];
				mesh_train.at<float>(i,1) = vertices_array[i][1];
			}
			//进行训练和预测
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

			for(int i = 0;i<candide3.nVertex();i++){
				vertices_array[i][0] = mesh_predict.at<float>(i,0);
				vertices_array[i][1] = mesh_predict.at<float>(i,1);
			}
			for(int i = 0;i<FFP_number;i++){
				vertices_array[FFP_index[i]][0] = LM_array.at<float>(i,0);
				vertices_array[FFP_index[i]][1] = LM_array.at<float>(i,1);
			}

			glutPostRedisplay();
			break;
		}
		default:
			cerr<<"Error in mesh deformation!"<<endl;
	}
}

void processMouse(int button, int state, int x, int y){
	//x,y是窗口像素点坐标
	float mouse[2];
    float origin_x = (window_width - viewport_width*viewport_scaler)/2;   //视口左上角像素x坐标
	float origin_y = (window_height - viewport_height*viewport_scaler)/2; //视口左上角像素y坐标
	float xx = (x-origin_x)/viewport_scaler; 
	float yy = (y-origin_y)/viewport_scaler;
	mouse[0] = (2*xx-viewport_width)/viewport_width;    //视口像素转为视口坐标
	mouse[1] = (viewport_height-2*yy)/viewport_height;  //视口像素转为视口坐标

	if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN){
		modelViewMatrix.PushMatrix();
		M3DMatrix44f mObjectFrame;
		objectFrame->GetMatrix(mObjectFrame);
		modelViewMatrix.MultMatrix(mObjectFrame);
		//左击鼠标，点击mesh vertex得到其对应索引
		float v[3], t[3];
		for(int i = 0; i < nVerts; i++){
			v[0] = vertices_array[i][0];
			v[1] = vertices_array[i][1];
			v[2] = vertices_array[i][2];
			m3dTransformVector3(t, v, transformPipeline.GetModelViewProjectionMatrix());
			if(pow(t[0]-mouse[0],2)+pow(t[1]-mouse[1],2)<0.001){
				cout<<"vertex: "<<i<<" ("<<t[0]<<','<<t[1]<<')'<<endl;
				break;
			}
		}
		modelViewMatrix.PopMatrix();
		//左击鼠标，点击landmark点得到其对应索引
		for(int i = 0; i<global_params.landmark_num; i++)
			if(pow(pixels_landmarks[i][0]-xx,2)+pow(pixels_landmarks[i][1]-yy,2)<10){
				cout<<"landmark: "<<i<<" ("<<landmarks[i][0]<<','<<landmarks[i][1]<<')'<<endl;
				break;
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

	viewport_scaler = width_scaler<height_scaler? width_scaler : height_scaler;
    float x = (window_width - viewport_width*viewport_scaler)/2;
	float y = (window_height - viewport_height*viewport_scaler)/2;

	glViewport(x, y, viewport_width*viewport_scaler, viewport_height*viewport_scaler);

	//视口坐标为正投影坐标
	//x: -ratio~ratio (-1~1)
	//y:-1~1 (-1/ratio~1/ratio)
	//z:-1~1
	if(ratio>1)
		viewFrustum.SetOrthographic(-ratio,ratio,-1,1,-1.0f,1.0f);
	else
		viewFrustum.SetOrthographic(-1,1,-1/ratio,1/ratio,-1.0f,1.0f);
	
	projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
	//modelViewMatrix.LoadIdentity();
}

void TimeFunc(int value){
	RenderScene();
	glutTimerFunc(40,TimeFunc,0);
}

///////////////////////////////////////////////////////////////////////////////
// Main entry point for GLUT based programs
int main(int argc, char* argv[]){

	/*string frame_type_string;

	if(frame_type_string=="image"){
		frame_type = IMAGE;
	}else if(frame_type_string=="camera"){
		frame_type = CAMERA;
		glutTimerFunc(40,TimeFunc,0);
	}else{
		cerr<<"invalid input!"<<endl;
		return -1;
	}*/

	gltSetWorkingDirectory(argv[0]);
	
	glutInit(&argc, argv);
	//glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(200, 200);
	glutCreateWindow("AutoFace 2.0");
    glutReshapeFunc(ChangeSize);
    glutKeyboardFunc(KeyPressFunc);
    glutSpecialFunc(SpecialKeys);
    glutDisplayFunc(RenderScene);

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
	glutAddMenuEntry("Open picture",1);
	glutAddMenuEntry("Open camera",2);
	glutAddMenuEntry("Detect landmarks",3);
	glutAddMenuEntry("Open wfm file",4);
	glutAddMenuEntry("Save wfm file",5);
	glutAddMenuEntry("Save off file",6);
	glutAddMenuEntry("Head Pose Estimation",7);
	glutAddSubMenu("Mesh Deformation",MeshDeformationMenu);

    glutAttachMenu(GLUT_RIGHT_BUTTON);
	//////////////////////////////////

    ////////////////////////////   
	//鼠标操作
	glutMouseFunc(processMouse);
	////////////////////////////

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