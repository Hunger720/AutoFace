//#include <math3d.h>
//
//class Unit{
//
//public:
//	Unit(){ Num = 0; }
//
//	//////////////////////////////////////////////////////////////////////////////////////
//	//inline void getName(char *name){ name = Name;}
//
//	inline int getNum(){return Num;}
//
//	int getIndex(int n){ if(n<Num) return Index[n]; }
//
//	void getStep(int n, M3DVector3f &s){
//		if(n<Num){
//			s[0] = Steps[n][0];
//			s[1] = Steps[n][1];
//			s[2] = Steps[n][2];
//		}
//	}
//
//	///////////////////////////////////////////////////////////////////////////////////////
//	//inline void setName(char *name){ Name = name; }
//
//	inline void setNum(int n){ Num = n; Index = new int[n]; Steps = new M3DVector3f[n]; }
//
//	void setIndex(int n,int i){ if(n<Num) Index[n] = i; }
//
//	void setStep(int n, M3DVector3f s){
//		if(n<Num){
//			Steps[n][0] = s[0];
//			Steps[n][1] = s[1];
//			Steps[n][2] = s[2];
//		}
//	}
//
//	////////////////////////////////////////////////////////////////////////////////////////
//
//private:
//	//char        *Name;
//	int         Num;
//	int         *Index;
//	M3DVector3f *Steps;
//};