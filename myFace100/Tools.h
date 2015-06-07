#include <math.h>

#include <math3d.h>

typedef float point[2];	

int clockwise(M3DVector3f v0, M3DVector3f v1, M3DVector3f v2){
	M3DVector3f vect0, vect1, result;
	m3dSubtractVectors3(vect0, v1, v0);
	m3dSubtractVectors3(vect1, v2, v1);
	m3dCrossProduct3(result, vect0, vect1);
	if(result[2] > 0)return 1;
	if(result[2] == 0)return 0;
	return -1;
}

float rollAngle(point *p){
	float R1, R2, R3;
    
	R1 = atan((p[0][1]-p[1][1])/(p[0][0]-p[1][0]));
	R2 = atan((p[2][1]-p[3][1])/(p[2][0]-p[3][0]));
	R3 = atan((p[4][1]-p[5][1])/(p[4][0]-p[5][0]));

	return (R1+R2+R3)/3;
}