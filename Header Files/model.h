#ifndef __MODEL_H__
#define __MODEL_H__

#include <GLTools.h>
#include "math3d.h"

class Unit{  //静态单元和动态单元的数据结构

public:
	Unit(){ Num = 0; }

	//////////////////////////////////////////////////////////////////////////////////////
	//inline void getName(char *name){ name = Name;}

	inline int getNum(){return Num;}

	int getIndex(int n){ if(n<Num) return Index[n]; }

	void getStep(int n, M3DVector3f &s){
		if(n<Num){
			s[0] = Steps[n][0];
			s[1] = Steps[n][1];
			s[2] = Steps[n][2];
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////
	//inline void setName(char *name){ Name = name; }

	inline void setNum(int n){ Num = n; Index = new int[n]; Steps = new M3DVector3f[n]; }

	void setIndex(int n,int i){ if(n<Num) Index[n] = i; }

	void setStep(int n, M3DVector3f s){
		if(n<Num){
			Steps[n][0] = s[0];
			Steps[n][1] = s[1];
			Steps[n][2] = s[2];
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////

private:
	int         Num;             //每个单元的顶点的数目
	int         *Index;          //顶点序号列表
	M3DVector3f *Steps;          //每个顶点对应的增量
};

class Model{

public:
	Model();

	inline int nVertex()const {return n_vertice;}
	inline int nFace(){return n_faces;};
	inline int nSUs(){return SUsNum;};
	inline int nAUs(){return AUsNum;};

	bool open(const char *file);                                  //读一个wfm文件
	bool write(const char *file);                                 //写一个wfm文件
	void copyVerticesData(int vertexnum, M3DVector3f *vertex);
	void copyFacesData(int facenum, M3DVector3i *face);
	void copySUsData(int susnum, Unit *sus);
	void copyAUsData(int ausnum, Unit *aus);
	void copySPsData(int spnum, float *sps);
	void copyAPsData(int apnum, float *aps);
    bool loadTexImage(const char *f);                             //载入纹理图片

	void          getVertex(int n, M3DVector3f &vertex) const;
	void          getTransCoords(int n, M3DVector3f &coord);
	void          getTexCoords(int n, float &x, float &y);
	void          getFace(int n, M3DVector3i &face);
	int           getSUNum(int n);
	int           getAUNum(int n);
	void          getSUIndex(int n, int *index);
	void          getAUIndex(int n, int *index);
	void          getSUSteps(int n, M3DVector3f *steps);
	void          getAUSteps(int n, M3DVector3f *steps);
	const char *  getSUsName(int n);
	const char *  getAUsName(int n);
	float         getSP(int n);
	float         getAP(int n);
	//inline int  getImageHeight(){return image.height();}
	//inline int  getImageWidth(){return image.width();}
	//GLbyte *    getTexData();

	void setVertex(int n, M3DVector3f vertex);
	void setTransCoords(int n, M3DVector3f coord);
	void setTexCoords(int n, float x, float y);
	void setFace(int n, M3DVector3i face);
	void setSU(int n, Unit su);
	void setAU(int n, Unit au);
	void setSUsName(int n, const char *name);
	void setAUsName(int n, const char *name);
	void setSP(int n, float sp);
	void setAP(int n, float ap);

	void addSP(int n, float sp);
	void addAP(int n, float ap);
	void applySP();             //TransCoords * SP → TransCoords
	void applyAP();             //TransCoords * AP → TransCoords
	//void updateModel();

	void clear();      //清空模型对象的所有数据
	void clearAP();
	~Model();

private:
	int          n_vertice;  //顶点数目
	int          n_faces;    //三角形面数目
	int          SUsNum;
	int          AUsNum;

	const char   **SUsName;
	const char   **AUsName;
	const char   *TexImage;         //纹理图片

	float        *SP;
	float        *AP;

	bool         TexExist;          //纹理图片是否有效

	M3DVector3f  *vertices;             //指向顶点数组
	M3DVector3f  *TransCoords;  //
	M3DVector3i  *faces;                //指向三角形面数组
	M3DVector2f  *texture_coordinates;  //指向二维纹理坐标数组
	Unit         *SU;
	Unit         *AU;

	//Image        image;
};

void makeCandide3Model(Model &candide3);
void makeSUs(Model &candide3);
void makeAUs(Model &candide3);

#endif