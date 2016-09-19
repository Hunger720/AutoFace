#ifndef _CLOCKWISE_H
#define _CLOCKWISE_H

/*计算三维空间上三角形的环绕方向（从z轴正方向往z轴负方向看）；
  输入参数为三角形三个顶点；
  逆时针返回值大于零，顺时针返回值小于零。*/
int clockwise(float *a, float *b, float *c);

#endif