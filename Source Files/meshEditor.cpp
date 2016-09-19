#include "meshEditor.h"

int myReadMesh(const char* path, M3DVector3f*& vertice_array, M3DVector3i*& faces_array, int& number_vertice, int& number_faces){
	int i, j;
	MyMesh mesh;

	//Read mesh from a .off file
	try{
		if (!OpenMesh::IO::read_mesh(mesh, path)){
			std::cerr << "Cannot write mesh to file 'output.off'" << std::endl;
			return 1;
		}
	}
	catch (std::exception& x){
		std::cerr << x.what() << std::endl;
		return 1;
	}
	
	//读取顶点数据
	number_vertice = mesh.n_vertices();
	vertice_array = new M3DVector3f[number_vertice];
	i = 0;
	for(MyMesh::VertexIter v_it=mesh.vertices_begin(); v_it!=mesh.vertices_end(); v_it++, i++){
		vertice_array[i][0] = mesh.point(*v_it)[0];
		vertice_array[i][1] = mesh.point(*v_it)[1];
		vertice_array[i][2] = mesh.point(*v_it)[2];
	}
	std::cout<<"vertice number = "<<mesh.n_vertices()<<std::endl;mesh.n_vertices();

	//读取面数据
	number_faces = mesh.n_faces();
	faces_array = new M3DVector3i[number_faces];
	i = 0;
	for(MyMesh::FaceIter f_it=mesh.faces_begin(); f_it!=mesh.faces_end(); f_it++, i++){
		j = 0;
		for(MyMesh::FaceVertexIter fv_it=mesh.fv_iter(*f_it); fv_it.is_valid(); fv_it++, j++){
			faces_array[i][j] = fv_it->idx();
		}
	}
	std::cout<<"faces number = "<<mesh.n_faces()<<std::endl;

	return 0;
}



int myWriteMesh(float *vertice, M3DVector3i* faces, int number_vertice, int number_faces){
	MyMesh mesh;

	// generate vertices
	MyMesh::VertexHandle *vhandle;
	vhandle = new MyMesh::VertexHandle[number_vertice];
	for(int i=0; i<number_vertice; i++){
		vhandle[i] = mesh.add_vertex(MyMesh::Point(vertice[i*3+0], vertice[i*3+1], vertice[i*3+2]));
	}
	
	// generate (quadrilateral) faces
	std::vector<MyMesh::VertexHandle>  face_vhandles;
	for(int i=0; i<number_faces; i++){
		face_vhandles.clear();
		face_vhandles.push_back(vhandle[faces[i][0]]);
		face_vhandles.push_back(vhandle[faces[i][1]]);
		face_vhandles.push_back(vhandle[faces[i][2]]);
		mesh.add_face(face_vhandles);
	}

	// write mesh to output.obj
	try{
		if (!OpenMesh::IO::write_mesh(mesh, "C:\\Users\\Administrator\\Desktop\\output.off")){
			std::cerr << "Cannot write mesh to file 'output.off'" << std::endl;
			return 1;
		}
	}
	catch (std::exception& x){
		std::cerr << x.what() << std::endl;
		return 1;
	}

	return 0;
}