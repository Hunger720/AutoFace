#ifndef __MESH_EDITOR_H__
#define __MESH_EDITOR_H__

#include <iostream>
#include "math3d.h"
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/PolyMesh_ArrayKernelT.hh>

typedef OpenMesh::PolyMesh_ArrayKernelT<>  MyMesh;

//读取.off文件，将点、面数据存储在数组中
int myReadMesh(const char* path, M3DVector3f*& vertice_array, M3DVector3i*& faces_array, int& number_vertice, int& number_faces);

//将数组中的点、面数据保存为.off文件
int myWriteMesh(float *vertice, M3DVector3i* face, int number_vertice, int number_faces);

#endif