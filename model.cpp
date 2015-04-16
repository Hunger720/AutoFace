#include "model.h"
#include "io.h"
#include <fstream>


Model::Model(){
	VertexNum = 0;
	FaceNum = 0;
	SUsNum = AUsNum = 0;
};

/////////////////////////////////////////////////////////////////////////////////////////////////
void Model::open(const char *f){
	int nVerts, nFaces;
	char line[100];
	char *title[] = {"# VERTEX LIST:", "# FACE LIST:", "# ANIMATION UNITS LIST:", "# SHAPE UNITS LIST:"};
	string t0(title[0]), t1(title[1]), t2(title[2]), t3(title[3]);

	M3DVector3f *vertex;
	M3DVector3i *face;

	ifstream file(f,ios::in);
	if(file){
		//读取Vertex数据
		do{
			file.getline(line,100);
		}while(t0.compare(line));

		file.getline(line,100);
		nVerts = char2int(line);

		vertex = new M3DVector3f[nVerts];
		for(int i = 0; i < nVerts; i++){
			file.getline(line,100);
			char2vertex(line,vertex[i][0],vertex[i][1],vertex[i][2]);
		}
		copyVertexData(nVerts,vertex);

		//读取Face数据
		do{
			file.getline(line,100);
		}while(t1.compare(line));

		file.getline(line,100);
		nFaces = char2int(line);

		face = new M3DVector3i[nFaces];
		for(int i = 0; i < nFaces; i++){
			file.getline(line,100);
			char2face(line,face[i][0],face[i][1],face[i][2]);
		}
		copyFaceData(nFaces,face);
		
		//读取AU数据
		const char **AUsName;
		int AUNum, nAU, *AUIndex;
		M3DVector3f *AUStep;
		Unit *AU;
		string *name;

		do{
			file.getline(line,100);
		}while(t2.compare(line));
		file.getline(line,100);
		AUNum = char2int(line);
				
		AU = new Unit[AUNum];
		AUsName = new const char*[AUNum];
		name = new string[AUNum];

		for(int i = 0; i < AUNum; i++){
			do{
				file.getline(line,100);
			}while(line[0] != '#');
			name[i].append(line);
			AUsName[i] = name[i].data();
			while(line[0] == '#')
				file.getline(line,100);
			nAU = char2int(line);
			AUIndex = new int[nAU];
			AUStep = new M3DVector3f[nAU];
			
			AU[i].setNum(nAU);
			for(int j = 0; j < nAU; j++){
				file.getline(line,100);
				char2UnitData(line,AUIndex[j],AUStep[j][0],AUStep[j][1],AUStep[j][2]);
				AU[i].setIndex(j,AUIndex[j]);
				AU[i].setStep(j,AUStep[j]);
			}
		}
		copyAUsData(AUNum,AU);

		//读取AUsName数据
		for(int i = 0; i < AUNum; i++)
			setAUsName(i,AUsName[i]);

		//delete name;
		//读取SU数据
		const char **SUsName;
		int SUNum, nSU, *SUIndex;
		M3DVector3f *SUStep;
		Unit *SU;

		do{
			file.getline(line,100);
		}while(t3.compare(line));
		file.getline(line,100);
		SUNum = char2int(line);
				
		SU = new Unit[SUNum];
		SUsName = new const char*[SUNum];
		name = new string[SUNum];

		for(int i = 0; i < SUNum; i++){
			do{
				file.getline(line,100);
			}while(line[0] != '#');
			name[i].append(line);
			SUsName[i] = name[i].data();
			while(line[0] == '#')
				file.getline(line,100);
			nSU = char2int(line);
			SUIndex = new int[nSU];
			SUStep = new M3DVector3f[nSU];
			
			SU[i].setNum(nSU);
			for(int j = 0; j < nSU; j++){
				file.getline(line,100);
				char2UnitData(line,SUIndex[j],SUStep[j][0],SUStep[j][1],SUStep[j][2]);
				SU[i].setIndex(j,SUIndex[j]);
				SU[i].setStep(j,SUStep[j]);
			}
		}
		copySUsData(SUNum,SU);

		//读取SUsName数据
		for(int i = 0; i < SUNum; i++)
			setSUsName(i,SUsName[i]);
	}

	file.close();
}

void Model::write(const char *f){
	ofstream file(f,ios::out);

	if(file){
		//写入Vertex数据
		file<<"# VERTEX LIST:"<<endl;
		file<<VertexNum<<endl;
		for(int i = 0; i < VertexNum; i++){
			file<<Vertex[i][0]<<' '<<Vertex[i][1]<<' '<<Vertex[i][2]<<endl;
		}
		file<<endl<<endl;

		//写入Face数据
		file<<"# FACE LIST:"<<endl;
		file<<FaceNum<<endl;
		for(int i = 0; i < FaceNum; i++){
			file<<Face[i][0]<<' '<<Face[i][1]<<' '<<Face[i][2]<<endl;
		}
		file<<endl<<endl;

		//写入AU数据
		file<<"# ANIMATION UNITS LIST:"<<endl;
		file<<AUsNum<<endl<<endl;
		for(int i = 0; i < AUsNum; i++){
			file<<AUsName[i]<<endl;
			file<<AU[i].getNum()<<endl;
			M3DVector3f s;
			for(int j = 0; j < AU[i].getNum(); j++){
				AU[i].getStep(j,s);
				file<<AU[i].getIndex(j)<<' '<<s[0]<<' '<<s[1]<<' '<<s[2]<<endl;
			}
			file<<endl;
		}
		file<<endl;

		//写入SU数据
		file<<"# SHAPE UNITS LIST:"<<endl;
		file<<SUsNum<<endl<<endl;
		for(int i = 0; i < SUsNum; i++){
			file<<SUsName[i]<<endl;
			file<<SU[i].getNum()<<endl;
			M3DVector3f s;
			for(int j = 0; j < SU[i].getNum(); j++){
				SU[i].getStep(j,s);
				file<<SU[i].getIndex(j)<<' '<<s[0]<<' '<<s[1]<<' '<<s[2]<<endl;
			}
			file<<endl;
		}
		file<<endl;
	}

	file.close();
}

void Model::copyVertexData(int vertexnum, M3DVector3f *vertex){
	VertexNum = vertexnum;
	Vertex = new M3DVector3f[VertexNum];
	TransCoords = new M3DVector3f[VertexNum];
	TexCoords = new M3DVector2f[VertexNum];
	for(int i = 0; i < VertexNum; i++){
		setVertex(i,vertex[i]);
		setTransCoords(i,vertex[i]);
		setTexCoords(i,0,0);
	}
}

void Model::copyFaceData(int facenum, M3DVector3i *face){
	FaceNum = facenum;
	Face = new M3DVector3i[FaceNum];
	for(int i = 0; i < FaceNum; i++)
		setFace(i,face[i]);
}

void Model::copySUsData(int susnum, Unit *sus){
	SUsNum = susnum;
	SU = new Unit[SUsNum];
	SP = new float[SUsNum];
	SUsName = new const char*[SUsNum];
	for(int i = 0; i < SUsNum; i++){
		setSU(i,sus[i]);
		SP[i] = 0;
	}
}

void Model::copyAUsData(int ausnum, Unit *aus){
	AUsNum = ausnum;
	AU = new Unit[AUsNum];
	AP = new float[AUsNum];
	AUsName = new const char*[AUsNum];
	for(int i = 0; i < AUsNum; i++){
		setAU(i,aus[i]);
		AP[i] = 0;
	}
}

void Model::copySPsData(int spnum, float *sps){
	if(spnum == SUsNum)
		for(int i = 0; i < SUsNum; i++)
			SP[i] = sps[i];
}

void Model::copyAPsData(int apnum, float *aps){
	if(apnum == AUsNum)
		for(int i = 0; i < AUsNum; i++)
			AP[i] = aps[i];
}

//bool Model::loadTexImage(const char *file){
//	return image.open(file);
//}


//////////////////////////////////////////////////////////////////////////////////////////////////
void Model::getVertex(int n, M3DVector3f &vertex){
	if(n<VertexNum){
		vertex[0] = Vertex[n][0];
		vertex[1] = Vertex[n][1];
		vertex[2] = Vertex[n][2];
	}else{
		vertex[0] = 0.0f;
		vertex[1] = 0.0f;
		vertex[2] = 0.0f;
	}
}

void Model::getTransCoords(int n, M3DVector3f &coord){
	if(n<VertexNum){
		coord[0] = TransCoords[n][0];
		coord[1] = TransCoords[n][1];
		coord[2] = TransCoords[n][2];
	}else{
		coord[0] = 0.0f;
		coord[1] = 0.0f;
		coord[2] = 0.0f;
	}
}

void Model::getTexCoords(int n, float &x, float &y){
	if(n<VertexNum){
		x = TexCoords[n][0];
		y = TexCoords[n][1];
	}else{
		x = 0.0f;
		y = 0.0f;
	}
}

void Model::getFace(int n, M3DVector3i &face){
	if(n<FaceNum){
		face[0] = Face[n][0];
		face[1] = Face[n][1];
		face[2] = Face[n][2];
	}else{
		face[0] = 0;
		face[1] = 0;
		face[2] = 0;
	}
}

int  Model::getSUNum(int n){
	if(n<SUsNum) return SU[n].getNum();
}

int  Model::getAUNum(int n){
	if(n<AUsNum) return AU[n].getNum();
}

void Model::getSUIndex(int n, int *index){
	if(n<SUsNum)
		for(int i = 0; i < SU[n].getNum(); i++)
			index[i] = SU[n].getIndex(i);
}

void Model::getAUIndex(int n, int *index){
	if(n<AUsNum)
		for(int i = 0; i < AU[n].getNum(); i++)
			index[i] = AU[n].getIndex(i);
}

void Model::getSUSteps(int n, M3DVector3f *steps){
	if(n<SUsNum)
		for(int i = 0; i < SU[n].getNum(); i++)
			SU[n].getStep(i,steps[i]);
}

void Model::getAUSteps(int n, M3DVector3f *steps){
	if(n<AUsNum)
		for(int i = 0; i < AU[n].getNum(); i++)
			AU[n].getStep(i,steps[i]);
}

const char *  Model::getSUsName(int n){
	if(n<SUsNum) return SUsName[n];
}

const char *  Model::getAUsName(int n){
	if(n<AUsNum) return AUsName[n];
}

float Model::getSP(int n){
	if(n<SUsNum) return SP[n];
}

float Model::getAP(int n){
	if(n<AUsNum) return AP[n];
}

//GLbyte * Model::getTexData(){
//	return image.genTexData();
//}


//////////////////////////////////////////////////////////////////////////////////////////////////////
void Model::setVertex(int n, M3DVector3f vertex){
	if(n<VertexNum){
		Vertex[n][0] = vertex[0];
		Vertex[n][1] = vertex[1];
		Vertex[n][2] = vertex[2];
	}
}

void Model::setTransCoords(int n, M3DVector3f coord){
	if(n<VertexNum){
		TransCoords[n][0] = coord[0];
		TransCoords[n][1] = coord[1];
		TransCoords[n][2] = coord[2];
	}
}

void Model::setTexCoords(int n, float x, float y){
	if(n<VertexNum){
		TexCoords[n][0] = x;
		TexCoords[n][1] = y;
	}
}

void Model::setFace(int n, M3DVector3i face){
	if(n<FaceNum){
		Face[n][0] = face[0];
		Face[n][1] = face[1];
		Face[n][2] = face[2];
	}
}

void Model::setSU(int n, Unit su){
	if(n<SUsNum){
		int num = su.getNum();
		SU[n].setNum(num);

		M3DVector3f s;
		for(int i = 0; i < num; i++){
			SU[n].setIndex(i,su.getIndex(i));
			su.getStep(i,s);
			SU[n].setStep(i,s);
		}
	}
}

void Model::setAU(int n, Unit au){
	if(n<AUsNum){
		int num = au.getNum();
		AU[n].setNum(num);

		M3DVector3f s;
		for(int i = 0; i < num; i++){
			AU[n].setIndex(i,au.getIndex(i));
			au.getStep(i,s);
			AU[n].setStep(i,s);
		}
	}
}

void Model::setSUsName(int n, const char *name){
	if(n<SUsNum){
		SUsName[n] = name;
	}
}

void Model::setAUsName(int n, const char *name){
	if(n<AUsNum){
		AUsName[n] = name;
	}
}

void Model::setSP(int n, float sp){
	if(n<SUsNum) SP[n] = sp;
}

void Model::setAP(int n, float ap){
	if(n<AUsNum) AP[n] = ap;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////
void Model::applySP(int n){
	if(n<SUsNum){
		M3DVector3f result, orignal, step;
		for(int j = 0; j < SU[n].getNum(); j++){
			getTransCoords(SU[n].getIndex(j), orignal);
			SU[n].getStep(j,step);
			step[0] *= SP[n]; step[1] *= SP[n]; step[2] *= SP[n];
			m3dAddVectors3(result, orignal, step);
			setTransCoords(SU[n].getIndex(j), result);
		}
	}
}

void Model::applyAP(int n){
	if(n<AUsNum){
	M3DVector3f result, orignal, step;
		for(int j = 0; j < AU[n].getNum(); j++){
			getTransCoords(AU[n].getIndex(j), orignal);
			AU[n].getStep(j,step);
			step[0] *= AP[n]; step[1] *= AP[n]; step[2] *= AP[n];
			m3dAddVectors3(result, orignal, step);
			setTransCoords(AU[n].getIndex(j), result);
		}
	}
}

//void Model::updateModel(){
//	for(int i = 0; i < VertexNum; i++){
//		Vertex[i][0] = TransCoords[i][0];
//		Vertex[i][1] = TransCoords[i][1];
//		Vertex[i][2] = TransCoords[i][2];
//	}
//}


////////////////////////////////////////////////////////////////////////////////////////////////////////
Model::~Model(){
	VertexNum = 0;
	FaceNum = 0;
	SUsNum = 0;
	AUsNum = 0;
	delete Vertex; Vertex = NULL;
	delete Face;   Face = NULL;
	delete SU;     SU = NULL;
	delete AU;     AU = NULL;
}