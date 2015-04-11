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
#include <math3d.h>
#include <iostream>

#include "image.h"
#include "model.h"

#include <math.h>
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
int    bgswitch = 0;
int    wfswitch = 0;

//float ap = 0;
float winWidth;
float winHeight;

int clockwise(M3DVector3f v0, M3DVector3f v1, M3DVector3f v2){
	M3DVector3f vect0, vect1, result;
	m3dSubtractVectors3(vect0, v1, v0);
	m3dSubtractVectors3(vect1, v2, v1);
	m3dCrossProduct3(result, vect0, vect1);
	if(result[2] > 0)return 1;
	if(result[2] == 0)return 0;
	return -1;
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

	glEnable(GL_DEPTH);
	glEnable(GL_CULL_FACE);

	objectFrame = new GLFrame;

	m3dScaleMatrix44(scaler, 1, 1, 1);
	
 	glGenTextures(1,texture);
	glBindTexture(GL_TEXTURE_2D,texture[0]);
	//loadBackgroundTex("C:\\Users\\HG\\Desktop\\pic\\myFace.jpg");
	//loadBackgroundTex("C:\\Users\\HG\\Desktop\\pic\\huang.jpg");
	//loadBackgroundTex("C:\\Users\\HG\\Desktop\\pic\\yuki.jpg");
	//loadBackgroundTex("C:\\Users\\HG\\Desktop\\pic\\8.jpg");
	//loadBackgroundTex("C:\\Users\\HG\\Desktop\\pic\\yamap.jpg");
	loadBackgroundTex("C:\\Users\\HG\\Desktop\\pic\\tamakin.jpg",winWidth,winHeight);
	//loadBackgroundTex("C:\\Users\\HG\\Desktop\\pic\\chiyaki.jpeg");
	//loadBackgroundTex("C:\\Users\\HG\\Desktop\\pic\\tomo.jpg");

	glutReshapeWindow(winWidth,winHeight);

	float x, y;
	if(winWidth>=winHeight) x = winWidth/winHeight, y = 1;
	else x = 1, y = winHeight/winWidth;

	//Background Texture
	bgBatch.Begin(GL_TRIANGLES,6,1);
	bgBatch.MultiTexCoord2f(0,0.0f,0.0f);
    bgBatch.Vertex3f(-x,y,0.0f);
	bgBatch.MultiTexCoord2f(0,0.0f,1.0f);
    bgBatch.Vertex3f(-x,-y,0.0f);
	bgBatch.MultiTexCoord2f(0,1.0f,0.0f);
    bgBatch.Vertex3f(x,y,0.0f);
	
	bgBatch.MultiTexCoord2f(0,0.0f,1.0f);
    bgBatch.Vertex3f(-x,-y,0.0f);
	bgBatch.MultiTexCoord2f(0,1.0f,1.0f);
    bgBatch.Vertex3f(x,-y,0.0f);
	bgBatch.MultiTexCoord2f(0,1.0f,0.0f);
    bgBatch.Vertex3f(x,y,0.0f);
	bgBatch.End();

	//////////////////////////////////////////////////////////////////////
	nVerts = candide3.nVertex();
	nFaces = candide3.nFace();

	Verts = new GLfloat[nVerts*3];
	for (int i = 0; i<nVerts; i++){
		candide3.getVertex(i,Vert);
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

	modelViewMatrix.PushMatrix();

		if(bgswitch == 0){
			shaderManager.UseStockShader(GLT_SHADER_TEXTURE_REPLACE, transformPipeline.GetModelViewProjectionMatrix(),0);
			bgBatch.Draw();
		} 

		M3DMatrix44f mObjectFrame;
		objectFrame->GetMatrix(mObjectFrame);
		modelViewMatrix.MultMatrix(mObjectFrame);
		modelViewMatrix.MultMatrix(scaler);

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

				/*if(oneline(texcoords[0],texcoords[1],texcoords[2])) { adjust(texcoords[0],texcoords[1],texcoords[2]); cout<<"fuck"<<endl;}

				for(int j = 0; j < 3; j++)
					candide3.setTexCoords(Faces[i][j],(texcoords[j][0]/x+1)/2,(-texcoords[j][1]/y+1)/2);*/
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
			candide3.setSP(nSP,0.1);
			candide3.applySP(nSP);
			break;
		case 2:
			/*ap += 0.5;
			candide3.setAP(0,ap);
			candide3.applyAP();*/
			candide3.setAP(nAP,0.1);
			candide3.applyAP(nAP);
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
			candide3.setSP(nSP,-0.1);
			candide3.applySP(nSP);
			break;
		case 2:
			/*ap -= 0.5;
			candide3.setAP(0,ap);
			candide3.applyAP();*/
			candide3.setAP(nAP,-0.1);
			candide3.applyAP(nAP);
			break;
		}
		for (int i = 0; i < nVerts; i++){
			candide3.getTransCoords(i,Vert);
			Verts[i*3+0] = Vert[0], Verts[i*3+1] = Vert[1], Verts[i*3+2] = Vert[2];
		}
	}

	//清零
	if(key == 48){
		for (int i = 0; i<nVerts; i++){
			candide3.getVertex(i,Vert);
			Verts[i*3+0] = Vert[0], Verts[i*3+1] = Vert[1], Verts[i*3+2] = Vert[2];
			candide3.setTransCoords(i,Vert);
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
		for(int i = 0; i < candide3.nAUs(); i++){
			candide3.setAP(i,0);
			//candide3.applyAP(i);
		}
		/*for (int i = 0; i<nVerts; i++){
			candide3.getTransCoords(i,Vert);
			Verts[i*3+0] = Vert[0], Verts[i*3+1] = Vert[1], Verts[i*3+2] = Vert[2];
		}*/
	}
                
    glutPostRedisplay();
	}

///////////////////////////////////////////////////////////////////////////////
void ProcessMainMenu(int value){
	menu = value;
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

///////////////////////////////////////////////////////////////////////////////
// Window has changed size, or has just been created. In either case, we need
// to use the window dimensions to set the viewport and the projection matrix.
void ChangeSize(int w, int h)
{
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
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
	glutInitWindowSize(500, 500);
	glutCreateWindow("Candide3");
    glutReshapeFunc(ChangeSize);
    glutKeyboardFunc(KeyPressFunc);
    glutSpecialFunc(SpecialKeys);
    glutDisplayFunc(RenderScene);

	//candide3.open("C:\\Users\\HG\\Desktop\\pic\\candide31.wfm");
	candide3.open("C:\\Users\\HG\\Desktop\\pic\\temp.wfm");
	//candide3.write("C:\\Users\\HG\\Desktop\\pic\\temp.wfm");
	//makeAUs(candide3);

	////////////////////////////////////////////////////////
	int SUMenu = glutCreateMenu(ProcessSUMenu);
	for(int i = 0; i < candide3.nSUs(); i++){
		glutAddMenuEntry(candide3.getSUsName(i),i);
	}

	int AUMenu = glutCreateMenu(ProcessAUMenu);
	for(int i = 0; i < candide3.nAUs(); i++){
		glutAddMenuEntry(candide3.getAUsName(i),i);
	}

	int BGMenu = glutCreateMenu(ProcessBGMenu);
	glutAddMenuEntry("on",0);
	glutAddMenuEntry("off",1);

	int WFMenu = glutCreateMenu(ProcessWFMenu);
	glutAddMenuEntry("on",0);
	glutAddMenuEntry("off",1);

	glutCreateMenu(ProcessMainMenu);
    glutAddMenuEntry("Zoom",0);
    glutAddSubMenu("Adjust SU",SUMenu);
	glutAddSubMenu("Change AU",AUMenu);
	glutAddSubMenu("Background",BGMenu);
	glutAddSubMenu("WireFrame",WFMenu);

    glutAttachMenu(GLUT_RIGHT_BUTTON);
    ////////////////////////////////////////////////////////////   

	GLenum err = glewInit();
	if (GLEW_OK != err) {
		fprintf(stderr, "GLEW Error: %s\n", glewGetErrorString(err));
		return 1;
		}

	SetupRC();

	glutMainLoop();
	return 0;
	}