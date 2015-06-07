#include <iostream>
#include <opencv2/opencv.hpp>
#include <GLTools.h>

using namespace cv;

class Image{
	public:
		bool open(const char *f){
			FILE *pFile;
			pFile = fopen(f,"rb");
			if(pFile == NULL) return false;
		
			int   count = -1;
			char  temp;
			do{
				count++;
				temp = f[count];
			}while(temp != '\0');

			count++;
			file = new char[count];
			for(int i = 0; i < count; i++)
				file[i] = f[i];
			
			return true;
		}

		 GLbyte * genTexData(){
			Mat img;
			img = imread(file);
			Height = img.rows; Width = img.cols;

			if(img.channels() == 3){
				GLbyte *pBits;
				pBits = (GLbyte*)malloc(Height*Width*3*sizeof(GLbyte));
				if(pBits == NULL) return NULL;

				memcpy(pBits,img.data,Height*Width*3);

				return pBits;
			}

		}

		inline int width(){return Width;}
		inline int height(){return Height;}

	private:
		char   *file;
		int    Width;
		int    Height;
};


bool Image2Tex(const char *file, float &winWidth, float &winHeight){
	GLbyte *pBits;

	int width, height;
	Image im;
	if(im.open(file)){
		pBits = im.genTexData();
		height = im.height();
		width = im.width();

	winWidth = width, winHeight = height;

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, pBits);

	glGenerateMipmap(GL_TEXTURE_2D);

	free(pBits);

	return true;

	}else
		return false;
}