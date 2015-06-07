//#include "model.h"
//#include "io.h"
//#include <fstream>
//
//void makeCandide3Model(Model &candide3){
//	int nVerts, nFaces;
//	char line[100];
//	char *title[] = {"# VERTEX LIST:", "# FACE LIST:", "# ANIMATION UNITS LIST:", "# SHAPE UNITS LIST:"};
//	string t0(title[0]), t1(title[1]), t2(title[2]), t3(title[3]);
//
//	M3DVector3f *vertex;
//	M3DVector3i *face;
//
//	ifstream file("C:\\Users\\HG\\Desktop\\pic\\candide3.wfm",ios::in);
//	if(file){
//		do{
//			file.getline(line,100);
//		}while(t0.compare(line));
//
//		file.getline(line,100);
//		nVerts = char2int(line);
//
//		vertex = new M3DVector3f[nVerts];
//		for(int i = 0; i < nVerts; i++){
//			file.getline(line,100);
//			char2vertex(line,vertex[i][0],vertex[i][1],vertex[i][2]);
//		}
//		candide3.copyVertexData(nVerts,vertex);
//
//		do{
//			file.getline(line,100);
//		}while(t1.compare(line));
//
//		file.getline(line,100);
//		nFaces = char2int(line);
//
//		face = new M3DVector3i[nFaces];
//		for(int i = 0; i < nFaces; i++){
//			file.getline(line,100);
//			char2face(line,face[i][0],face[i][1],face[i][2]);
//		}
//		candide3.copyFaceData(nFaces,face);
//		/////////////////////////////////////////////////////////////////////
//		const char **SUsName;
//		int SUNum, nSU, *SUIndex;
//		M3DVector3f *SUStep;
//		Unit *SU;
//		string *name;
//
//		do{
//			file.getline(line,100);
//		}while(t3.compare(line));
//		file.getline(line,100);
//		SUNum = char2int(line);
//		
//		SUsName = new const char*[SUNum];
//		SU = new Unit[SUNum];
//		name = new string[SUNum];
//
//		for(int i = 0; i < SUNum; i++){
//			do{
//				file.getline(line,100);
//			}while(line[0] != '#');
//			name[i].append(line);
//			SUsName[i] = name[i].data();
//			while(line[0] == '#')
//				file.getline(line,100);
//			nSU = char2int(line);
//			SUIndex = new int[nSU];
//			SUStep = new M3DVector3f[nSU];
//			
//			SU[i].setNum(nSU);
//			for(int j = 0; j < nSU; j++){
//				file.getline(line,100);
//				char2UnitData(line,SUIndex[j],SUStep[j][0],SUStep[j][1],SUStep[j][2]);
//				SU[i].setIndex(j,SUIndex[j]);
//				SU[i].setStep(j,SUStep[j]);
//			}
//		}
//
//		candide3.copySUsData(SUNum,SU);
//		for(int i = 0; i < SUNum; i++)
//			candide3.setSUsName(i,SUsName[i]);
//	}
//	////////////////////////////////////////////////////////////////////////////
//	
//	makeAUs(candide3);
//}
//
//void makeAUs(Model &candide3){
//	int num = 14;
//	int index[] = { 31, 64, 88, 89, 79, 80, 81, 
//					82, 83, 84, 85, 86, 90, 91   };
//	M3DVector3f steps[14] = {  0.000000,   -0.140000,   -0.010000,
//							   0.000000,   -0.140000,   -0.010000,
//							   0.000000,   -0.100000,   -0.008000,
//							   0.000000,   -0.100000,   -0.008000,
//							   0.000000,   -0.030000,   -0.020000,
//							   0.000000,   -0.030000,   -0.020000,
//							   0.000000,   -0.030000,   -0.020000,
//							   0.000000,   -0.030000,   -0.020000,
//							   0.000000,   -0.030000,   -0.020000,
//							   0.000000,   -0.030000,   -0.020000,
//							   0.000000,   -0.040000,   -0.020000,
//							   0.000000,   -0.040000,   -0.020000,
//							   0.000000,   -0.040000,   -0.000000,
//							   0.000000,   -0.040000,   -0.000000  };
//	Unit u;
//	u.setNum(num);
//	for(int i = 0; i < num; i++){
//		u.setIndex(i,index[i]);
//		u.setStep(i,steps[i]);
//	}
//
//	candide3.copyAUsData(1,&u);
//}