#ifndef _RBF_H
#define _RBF_H

#include<opencv2/opencv.hpp>
#include<cmath>

using namespace std;
using namespace cv;

enum KERNEL_TYPE{GAUSS, REFLECTED_SIGMOIDAL, INVERSE_MULTIQUADRICS};


/*这个径向基函数类只针对二维网格变形*/
class RBF{

public:
	RBF();
	bool train(InputArray src, InputArray dst);
	bool predict(InputArray src, OutputArray dst);
	float kernel(double x);
	inline void set_kernel_type(KERNEL_TYPE kt){ kernel_type = kt; }
	inline void set_sigma(double s){ sigma = s; cout<<sigma<<endl;}

private:
	double sigma;
	int centers_num;  //隐含层的维度
	int dim;          //输入输出层的维度
	KERNEL_TYPE kernel_type;
	Mat centers;      //中心点
	Mat omigas;       //权值
};

#endif