#include<opencv2/opencv.hpp>
#include<cmath>

using namespace std;
using namespace cv;

enum KERNEL_TYPE{GAUSS, REFLECTED_SIGMOIDAL, INVERSE_MULTIQUADRICS};

/*这个径向基函数类只针对二维网格变形*/
class RBF{
public:
	double sigma;
	int centers_num;  //隐含层的维度
	int dim;          //输入输出层的维度

	KERNEL_TYPE kernel_type;

	Mat centers;      //中心点
	Mat omigas;       //权值

	RBF();
	bool train(InputArray src, InputArray dst);
	bool predict(InputArray src, OutputArray dst);
	inline void set_kernel_type(KERNEL_TYPE kt){ kernel_type = kt; }
	inline void set_sigma(double s){ sigma = s; cout<<sigma<<endl;}
	float kernel(double x);
};

RBF::RBF(){
	sigma = 0.07;
	centers_num = 0;
	dim = 0;
	kernel_type = GAUSS;
}

bool RBF::train(InputArray src, InputArray dst){
	if(src.getMat().dims != 2 || dst.getMat().dims != 2){
		cerr<<"Invalid Input!"<<endl;
		return false;
	}else if(src.size().width == dst.size().width && src.size().height == dst.size().height){
		dim = src.size().width;
		centers_num = src.size().height;
		centers = dst.getMat();            //样本点矩阵

		Mat centers_trans = centers - src.getMat();   //计算样本点offset
		Mat train_coeff_matrix(centers_num, centers_num, CV_32F);
		Mat result;
		for(int i = 0;i<centers_num;i++)
			for(int j = 0;j<centers_num;j++)
				train_coeff_matrix.at<float>(i,j) = kernel(norm(centers.row(i),centers.row(j),NORM_L2));

		solve(train_coeff_matrix,centers_trans,result);  //计算权值

		omigas = result;

		return true;
	}else{
		cerr<<"Invalid Input!"<<endl;
		return false;
	}
}

bool RBF::predict(InputArray src, OutputArray dst){
	if(src.getMat().dims != 2){
		cerr<<"Invalid Input!"<<endl;
		return false;
	}else if(src.size().width == dim){
		Mat predict_coeff_matrix(src.size().height,centers_num, CV_32F);
		for(int i = 0;i<src.size().height;i++)
			for(int j = 0;j<centers_num;j++)
				predict_coeff_matrix.at<float>(i,j) = kernel(norm(src.getMat().row(i), centers.row(j),NORM_L2));

		Mat result = src.getMat() + predict_coeff_matrix * omigas;
		result.copyTo(dst);

		return true;
	}else{
		cerr<<"Invalid Input!"<<endl;
		return false;
	}
}

float RBF::kernel(double x){
	switch(kernel_type){
	case GAUSS:
		return (float)(exp(-x*x/(2*sigma*sigma)));
		break;
	case REFLECTED_SIGMOIDAL:
		return (float)1/(1+exp(x*x/(sigma*sigma)));
		break;
	case INVERSE_MULTIQUADRICS:
		return (float)1/pow(x*x+sigma*sigma,0.5);
		break;
	default: 
		cerr<<"Error in kernel function!"<<endl;
		return -1;
	}
};