#include "clockwise.h"

int clockwise(float *a, float *b, float *c){
	float vector1[3], vector2[3], cross_product[3];

	for(int i = 0; i<3; i++){
		vector1[i] = b[i] - a[i];
		vector2[i] = c[i] - b[i];
	}

	cross_product[0] = vector1[1]*vector2[2] - vector1[2]*vector2[1];
	cross_product[1] = vector1[2]*vector2[0] - vector1[0]*vector2[2];
	cross_product[2] = vector1[0]*vector2[1] - vector1[1]*vector2[0];

	if(cross_product[2] > 0)return 1;
	if(cross_product[2] < 0)return -1;
	return 0;
}