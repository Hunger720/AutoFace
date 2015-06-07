// Primitieves.cpp
// OpenGL SuperBible, Chapter 2
// Demonstrates the 7 Geometric Primitives
// Program by Richard S. Wright Jr.

#include <GLTools.h>	// OpenGL toolkit
#include <GLMatrixStack.h>
#include <GLFrame.h>
#include <GLFrustum.h>
#include <GLBatch.h>
#include <GLGeometryTransform.h>
#include <iostream>

#include "image.h"
#include "model.h"
#include "Tools.h"

#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <glut.h>
#endif

using namespace std;

/////////////////////////////////////////////////////////////////////////////////
// An assortment of needed classes
GLShaderManager		shaderManager;
GLMatrixStack		modelViewMatrix;
GLMatrixStack		projectionMatrix;
GLFrame             *objectFrame;
GLFrustum			viewFrustum;

GLBatch             bgBatch;                //背景图片
GLBatch             *wireframeBatch;        //线框模型
GLBatch				*facesBatch;            //贴图模型

GLGeometryTransform	transformPipeline;

GLuint texture[1];
GLuint nVerts;
GLuint nFaces;

GLfloat vWhite[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat *Verts;
GLfloat texcoords[3][3];

M3DVector3i *Faces;
M3DVector3f Face[3];
M3DVector3f Vert;
M3DMatrix44f scaler;

Model candide3;

// Keep track of effects step
int    nStep = 0;
int    menu = 0;
int    nSP = 0;
int    nAP = 0;
int    bgswitch = 1;    //背景图片显示开关
int    wfswitch = 0;    //线框模型显示开关
int    ffpcount = 0;    //

const int    ffpNum   = 11;   //人脸特征点数目

//float ap = 0;
float  winWidth;
float  winHeight;
float  winW;            //实际上的窗口宽度（像素数）
float  winH;            //实际上的窗口高度（像素数）
float  r;               //人脸旋转偏量
float  s;               //人脸缩放因子
//float  mouse[2];        //鼠标坐标

point  t;               //人脸位移偏量
point  ffp[ffpNum];             //人脸特征点

const char *teximage = "C:\\Users\\HG\\Desktop\\pic\\tamakin.jpg";
//const char *teximage = "C:\\Users\\HG\\Desktop\\pic\\tomo.jpg";
//const char *teximage = "C:\\Users\\HG\\Desktop\\pic\\yamap.jpg";

bool findVertex(float x, float y, float vertex[3]){
	float t[3];

	m3dTransformVector3(t, vertex, transformPipeline.GetModelViewMatrix());

	if(pow(t[0]-x,2)+pow(t[1]-y,2)<0.001)return true;
	else return false;
}

//bool oneline(M3DVector3f v0, M3DVector3f v1, M3DVector3f v2){
//	bool result = false;
//	if(abs((v1[1]-v0[1])*(v2[0]-v0[0]) - (v2[1]-v0[1])*(v1[0]-v0[0])) < 0.0001)
//		result = true;
//	return result;
//}
//
//void adjust(M3DVector3f &v0, M3DVector3f &v1, M3DVector3f &v2){
//	/*if(v0[0]>v1[0]){
//		if(v0[0]>v2[0]) v0[1] += 0.1;
//		else v2[1] += 0.1;
//	}else{
//		if(v1[0]>v2[0]) v1[1] += 0.1;
//		else v2[1] += 0.1;
//	}*/
//	v0[1] += 0.1;
//}


///////////////////////////////////////////////////////////////////////////////
// This function does any needed initialization on the rendering context. 
// This is the first opportunity to do any OpenGL related tasks.
void SetupRC()
{
    // Black background
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f );

	shaderManager.InitializeStockShaders();

	transformPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	objectFrame = new GLFrame;

	m3dScaleMatrix44(scaler, 1, 1, 1);
	
 	glGenTextures(1,texture);
	glBindTexture(GL_TEXTURE_2D,texture[0]);
	//loadBackgroundTex("C:\\Users\\HG\\Desktop\\pic\\myFace.jpg");
	//loadBackgroundTex("C:\\Users\\HG\\Desktop\\pic\\huang.jpg");
	//loadBackgroundTex("C:\\Users\\HG\\Desktop\\pic\\yuki.jpg");
	//loadBackgroundTex("C:\\Users\\HG\\Desktop\\pic\\8.jpg");
	Image2Tex(teximage,winWidth,winHeight);    //加载背景图片
	//loadBackgroundTex("C:\\Users\\HG\\Desktop\\pic\\chiyaki.jpeg");

	glutReshapeWindow(winWidth,winHeight);

	float x, y;
	if(winWidth>=winHeight) x = winWidth/winHeight, y = 1;
	else x = 1, y = winHeight/winWidth;

	//Background Texture
	bgBatch.Begin(GL_TRIANGLES,6,1);
	bgBatch.MultiTexCoord2f(0,0.0f,0.0f);
    bgBatch.Vertex3f(-x,y,-.5f);
	bgBatch.MultiTexCoord2f(0,0.0f,1.0f);
    bgBatch.Vertex3f(-x,-y,-.5f);
	bgBatch.MultiTexCoord2f(0,1.0f,0.0f);
    bgBatch.Vertex3f(x,y,-.5f);
	
	bgBatch.MultiTexCoord2f(0,0.0f,1.0f);
    bgBatch.Vertex3f(-x,-y,-.5f);
	bgBatch.MultiTexCoord2f(0,1.0f,1.0f);
    bgBatch.Vertex3f(x,-y,-.5f);
	bgBatch.MultiTexCoord2f(0,1.0f,0.0f);
    bgBatch.Vertex3f(x,y,-.5f);
	bgBatch.End();

	//////////////////////////////////////////////////////////////////////
	nVerts = candide3.nVertex();
	nFaces = candide3.nFace();

	Verts = new GLfloat[nVerts*3];

	candide3.applySP();
	candide3.applyAP();
	for (int i = 0; i<nVerts; i++){
		candide3.getTransCoords(i,Vert);
		Verts[i*3+0] = Vert[0], Verts[i*3+1] = Vert[1], Verts[i*3+2] = Vert[2];
	}

	Faces = new M3DVector3i[nFaces];
	for (int i = 0; i<nFaces; i++)
		candide3.getFace(i,Faces[i]);
	
}


///////////////////////////////////////////////////////////////////////////////
// Called to draw scene
void RenderScene(void)
{    
	// Clear the window with current clearing color
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	

	modelViewMatrix.PushMatrix();

		if(bgswitch == 1){
			shaderManager.UseStockShader(GLT_SHADER_TEXTURE_REPLACE, transformPipeline.GetModelViewProjectionMatrix(),0);
			bgBatch.Draw();
		} 

		M3DMatrix44f mObjectFrame;
		objectFrame->GetMatrix(mObjectFrame);
		modelViewMatrix.MultMatrix(mObjectFrame);
		modelViewMatrix.MultMatrix(scaler);

	/*	M3DMatrix44f m;
		m3dLoadIdentity44(m);*/
		M3DVector3f t[3];

		//每次渲染三角形前调整环绕方向
		for (int i = 0; i<nFaces; i++){
			Face[0][0] = Verts[Faces[i][0]*3+0];
			Face[0][1] = Verts[Faces[i][0]*3+1];
			Face[0][2] = Verts[Faces[i][0]*3+2];

			Face[1][0] = Verts[Faces[i][1]*3+0];
			Face[1][1] = Verts[Faces[i][1]*3+1];
			Face[1][2] = Verts[Faces[i][1]*3+2];

			Face[2][0] = Verts[Faces[i][2]*3+0];
			Face[2][1] = Verts[Faces[i][2]*3+1];
			Face[2][2] = Verts[Faces[i][2]*3+2];

			/*m3dTransformVector3(t[0], Face[0], transformPipeline.GetModelViewMatrix());
			m3dTransformVector3(t[1], Face[1], transformPipeline.GetModelViewMatrix());
			m3dTransformVector3(t[2], Face[2], transformPipeline.GetModelViewMatrix());

			if(clockwise(t[0],t[1],t[2])>=0){
				int temp;
				temp = Faces[i][0];
				Faces[i][0] = Faces[i][1];
				Faces[i][1] = temp;
			}*/
			if(clockwise(Face[0],Face[1],Face[2])>=0){
				int temp;
				temp = Faces[i][0];
				Faces[i][0] = Faces[i][1];
				Faces[i][1] = temp;
			}
		}

		//////////////////////////////////////////////////////////
		delete wireframeBatch;
		wireframeBatch = NULL;
		wireframeBatch = new GLBatch;
		wireframeBatch->Begin(GL_TRIANGLES, nFaces*3);
		for(int i = 0;i<nFaces; i++){
			wireframeBatch->Vertex3f(Verts[Faces[i][0]*3+0],Verts[Faces[i][0]*3+1],Verts[Faces[i][0]*3+2]);
			wireframeBatch->Vertex3f(Verts[Faces[i][1]*3+0],Verts[Faces[i][1]*3+1],Verts[Faces[i][1]*3+2]);
			wireframeBatch->Vertex3f(Verts[Faces[i][2]*3+0],Verts[Faces[i][2]*3+1],Verts[Faces[i][2]*3+2]);
		};
		wireframeBatch->End();
		/////////////////////////////////////////////////////////
		delete facesBatch;
		facesBatch = NULL;
		facesBatch = new GLBatch;
		facesBatch->Begin(GL_TRIANGLES, nFaces*3,1);  
		for(int i = 0; i < nFaces; i++){
			for(int j = 0; j < 3; j++){
				candide3.getTexCoords(Faces[i][j],texcoords[j][0],texcoords[j][1]);
				facesBatch->MultiTexCoord2f(0,texcoords[j][0],texcoords[j][1]);
				//cout<<texcoords[0]<<'\t'<<texcoords[1]<<endl;
				facesBatch->Vertex3f(Verts[Faces[i][j]*3+0],Verts[Faces[i][j]*3+1],Verts[Faces[i][j]*3+2]);
			}
		}
		facesBatch->End();

		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vWhite);

		switch(nStep) {
			case 0:
				break;
			case 1:
				shaderManager.UseStockShader(GLT_SHADER_TEXTURE_REPLACE, transformPipeline.GetModelViewProjectionMatrix(),0);
				facesBatch->Draw();
				break;
			}

		if(wfswitch == 0){
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			wireframeBatch->Draw();
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

	modelViewMatrix.PopMatrix();

	// Flush drawing commands
	glutSwapBuffers();
   }


// Respond to arrow keys by moving the camera frame of reference
void SpecialKeys(int key, int x, int y)
    {
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
void KeyPressFunc(unsigned char key, int x, int y)
{

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

	//Enter键
	if(key == 13){
		nStep = 1;
		modelViewMatrix.PushMatrix();

			M3DMatrix44f mObjectFrame;
			objectFrame->GetMatrix(mObjectFrame);
			modelViewMatrix.MultMatrix(mObjectFrame);
			modelViewMatrix.MultMatrix(scaler);

			GLfloat t[3];
			float x, y;
			if(winWidth>=winHeight) x = winWidth/winHeight, y = 1;
			else x = 1, y = winHeight/winWidth;
			for(int i = 0; i < nFaces; i++){
				for(int j = 0; j < 3; j++){
					t[0] = Verts[Faces[i][j]*3+0];
					t[1] = Verts[Faces[i][j]*3+1];
					t[2] = Verts[Faces[i][j]*3+2];
					m3dTransformVector3(texcoords[j], t, transformPipeline.GetModelViewMatrix());
					candide3.setTexCoords(Faces[i][j],(texcoords[j][0]/x+1)/2,(-texcoords[j][1]/y+1)/2);
				}
			}

		modelViewMatrix.PopMatrix();

	}
	
	//空白键
	if(key == 32){
		nStep++;
		if(nStep > 1)
			nStep = 0;
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
			candide3.getTransCoords(i,Vert);
			Verts[i*3+0] = Vert[0], Verts[i*3+1] = Vert[1], Verts[i*3+2] = Vert[2];
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
			candide3.getTransCoords(i,Vert);
			Verts[i*3+0] = Vert[0], Verts[i*3+1] = Vert[1], Verts[i*3+2] = Vert[2];
		}
	}

	//清零,回到标准Candide-3模型
	if(key == 48){
		for (int i = 0; i<nVerts; i++){
			candide3.getVertex(i,Vert);
			candide3.setTransCoords(i,Vert);
			Verts[i*3+0] = Vert[0], Verts[i*3+1] = Vert[1], Verts[i*3+2] = Vert[2];			
		}
		m3dScaleMatrix44(scaler, 1, 1, 1);
		delete objectFrame; objectFrame = NULL;
		objectFrame = new GLFrame;
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

		for (int i = 0; i<nVerts; i++){
			candide3.getTransCoords(i,Vert);
			Verts[i*3+0] = Vert[0], Verts[i*3+1] = Vert[1], Verts[i*3+2] = Vert[2];
		}

	}
                
    glutPostRedisplay();
	}

///////////////////////////////////////////////////////////////////////////////
void ProcessMainMenu(int value){
	float ss;

	switch (value){
	case 0:
		menu = value;
		break;
	case 1:
		candide3.loadTexImage("teximage");
		break;
	case 2:
		candide3.write("C:\\Users\\HG\\Desktop\\pic\\candide3.wfm");
		break;
	case 3:
		ffpcount = ffpNum;
		break;
	case 4:
		//cout<<rollAngle(ffp)<<endl;
		//objectFrame->RotateWorld(rollAngle(ffp), 0.0f, 0.0f, 1.0f);     //计算偏转量

		M3DVector3f v1,v2,t1,t2;
		candide3.getVertex(29, v1);
		candide3.getVertex(62, v2);
		//m3dTransformVector3(t1, v1, transformPipeline.GetModelViewMatrix());
		//m3dTransformVector3(t2, v2, transformPipeline.GetModelViewMatrix());
		//ss = sqrt(pow(ffp[7][0]-ffp[8][0],2)+pow(ffp[7][1]-ffp[8][1],2))/sqrt(pow(t1[0]-t2[0],2)+pow(t1[1]-t2[1],2));
		ss = sqrt(pow(ffp[9][0]-ffp[10][0],2)+pow(ffp[9][1]-ffp[10][1],2))/sqrt(pow(v1[0]-v2[0],2)+pow(v1[1]-v2[1],2));
		M3DMatrix44f s;
		m3dScaleMatrix44(s, ss, ss, ss);
		//cout<<ss<<endl;
		m3dMatrixMultiply44(scaler,scaler,s);    //计算缩放因子

		M3DVector3f v[3],t;
		candide3.getVertex(3, v[0]);
		candide3.getVertex(6, v[1]);
		candide3.getVertex(8, v[2]);
		//m3dTransformVector3(t, v, transformPipeline.GetModelViewMatrix());
		t[0] = (ffp[6][0]+ffp[7][0]+ffp[8][0]-v[0][0]-v[1][0]-v[2][0])/3;
		t[1] = (ffp[6][1]+ffp[7][1]+ffp[8][1]-v[0][1]-v[1][1]-v[2][1])/3;
		objectFrame->TranslateWorld(t[0], t[1], 0.0f);     //计算位移量

		/*Verts[15*3+0] = ffp[5][0];Verts[15*3+1] = ffp[5][1];
		Verts[17*3+0] = ffp[4][0];Verts[17*3+1] = ffp[4][1];*/

		objectFrame->RotateWorld(rollAngle(ffp), 0.0f, 0.0f, 1.0f);     //计算偏转量


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

void ProcessBGMenu(int value){
	bgswitch = value;
	glutPostRedisplay();
}

void ProcessWFMenu(int value){
	wfswitch = value;
	glutPostRedisplay();
}

void processMouse(int button, int state, int x, int y){
	float xx =x, yy = y, mouse[2];
	if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN){
		if(winWidth>=winHeight){
			mouse[0] = (2*xx - winW)/winH;
			mouse[1] = 1 - 2*yy/winH;
		}else{
			mouse[0] = 2*xx/winW - 1;
			mouse[1] = (winH-2*yy)/winW;
			//cout<<mouse[0]<<','<<mouse[1]<<endl;
		}
	
		if(ffpcount == 0){
			float t[3];
			for(int i = 0; i < nVerts; i++){
				t[0] = Verts[i*3+0];
				t[1] = Verts[i*3+1];
				t[2] = Verts[i*3+2];
				if(findVertex(mouse[0],mouse[1],t))cout<<i<<endl;
			}
		}else{
			ffpcount--;
			ffp[ffpcount][0] = mouse[0];
			ffp[ffpcount][1] = mouse[1];
			//cout<<mfcount<<endl;
			cout<<ffp[ffpcount][0]<<","<<ffp[ffpcount][1]<<endl;
		}

	}
}

///////////////////////////////////////////////////////////////////////////////
// Window has changed size, or has just been created. In either case, we need
// to use the window dimensions to set the viewport and the projection matrix.
void ChangeSize(int w, int h)
{
	winW = w; winH = h;
	float a = w , b = h, ratio;
	glViewport(0, 0, w, h);
	if(w>=h){
		ratio = a/b;
		viewFrustum.SetOrthographic(-ratio,ratio,-1,1,-1.0f,1.0f);
	}else{
		ratio = b/a;
		viewFrustum.SetOrthographic(-1,1,-ratio,ratio,-1.0f,1.0f);
	}
	projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
	modelViewMatrix.LoadIdentity();
}

///////////////////////////////////////////////////////////////////////////////
// Main entry point for GLUT based programs
int main(int argc, char* argv[])
{
	gltSetWorkingDirectory(argv[0]);
	
	glutInit(&argc, argv);
	//glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(500, 500);
	glutCreateWindow("Candide3");
    glutReshapeFunc(ChangeSize);
    glutKeyboardFunc(KeyPressFunc);
    glutSpecialFunc(SpecialKeys);
    glutDisplayFunc(RenderScene);

	//candide3.open("C:\\Users\\HG\\Desktop\\pic\\candide3.wfm");
	//candide3.open("C:\\Users\\HG\\Desktop\\pic\\temp.wfm");
	candide3.open("C:\\Users\\HG\\Desktop\\pic\\tamakin.wfm");
	//candide3.write("C:\\Users\\HG\\Desktop\\pic\\temp.wfm");
	//makeAUs(candide3);

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

	int BGMenu = glutCreateMenu(ProcessBGMenu);
	glutAddMenuEntry("on",1);
	glutAddMenuEntry("off",0);

	int WFMenu = glutCreateMenu(ProcessWFMenu);
	glutAddMenuEntry("on",0);
	glutAddMenuEntry("off",1);

	glutCreateMenu(ProcessMainMenu);
    glutAddMenuEntry("Zoom",0);
    glutAddSubMenu("Adjust SU",SUMenu);
	glutAddSubMenu("Change AU",AUMenu);
	glutAddSubMenu("Background",BGMenu);
	glutAddSubMenu("WireFrame",WFMenu);
	glutAddMenuEntry("Load Texture Image",1);
	glutAddMenuEntry("Save Model",2);
	glutAddMenuEntry("Select FFPs",3);
	glutAddMenuEntry("Global Motion",4);

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