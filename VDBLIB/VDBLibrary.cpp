#include "pch.h"
#include "VDBLibrary.h"


VDBLIB_API VDBO* CreateEmptyVDBO()
{
	VDBO* vdbo = new VDBO();
	return vdbo;
}

VDBLIB_API VDBO* CreateVDBO(const char* file)
{
	VDBO* vdbo = new VDBO(file);
	return vdbo;
}

VDBLIB_API VDBO* CreateSampleVDBO() {
	
	VDBO *vdbo = new VDBO();
	vdbo->initSampleCube(0.1f,1.0f,0.0f,0.0f,0.0f);
	/*std::cout << "___VDBO Created:___" << std::endl;
	std::cout << vdbo->mInfo << std::endl;
	std::cout << "___________________" << std::endl;*/

	return vdbo;
}

VDBLIB_API VDBO* CreateSampleVDBOCustom(float vxlSize, float dim, float posx, float posy, float posz)
{
	VDBO* vdbo = new VDBO();
	vdbo->initSampleCube(vxlSize,dim,posx,posy,posz);
	return vdbo;
}

VDBLIB_API VDBO* ChangeToSphereVDBO(VDBO* vdbo) {
	vdbo->changeSampleToSphere();
	return vdbo;
}

VDBLIB_API VDBO* CreateVDBOFromMesh(VDBO* vdbo, float* verts, int vCount, int* faces, int fCount, double voxelSize, double bandwidth){
	
	vdbo->clearPolygonsUtil();

	for (int vi = 0; vi < vCount; vi+=3) {
		openvdb::Vec3s thisV(verts[vi + 0], verts[vi + 1], verts[vi + 2]);
		vdbo->mPoints.push_back(thisV);
	}
	/*
	vdbo->mPoints.reserve(vCount);
	auto ptsIter = vdbo->mPoints.begin();
	tbb::parallel_for(tbb::blocked_range<int>(0, vCount),
		[&](tbb::blocked_range<int> r) {
			for (int vi = r.begin(); vi < r.end(); vi += 3) {
				openvdb::Vec3s thisV(verts[vi + 0], verts[vi + 1], verts[vi + 2]);
				vdbo->mPoints.insert(ptsIter + (vi / 3), thisV);
			}
		});
	*/
	for (int fi = 0; fi < fCount; fi+=3) {
		openvdb::Vec3I thisF(faces[fi + 0], faces[fi + 1], faces[fi + 2]);
		vdbo->mTriangles.push_back(thisF);
	}

	//vdbo->mTriangles.reserve(fCount);

	vdbo->voxelizePolygons(voxelSize, bandwidth);

	return vdbo;
}


//VDBLIB_API VDBO* CreateVDBO(const char* filepath) {
//	std::string fp(filepath);
//	VDBO *vdbo = new VDBO(fp);
//
//	/*std::cout << "___VDBO Created:___" << std::endl;
//	std::cout << vdbo->mInfo << std::endl;
//	std::cout << "___________________" << std::endl;*/
//
//	return vdbo;
//}

VDBLIB_API char* GetVDBOInfo(VDBO* vdbo) {
	

	int dest_size = vdbo->mInfo.length() + 1;
	
	char* result = new char[dest_size];

	strcpy_s(result, dest_size,vdbo->mInfo.c_str());
	
	return result;
}

VDBLIB_API char* VDBLibErrorInfo()
{
	//TODO
	return nullptr;
}

VDBLIB_API void PolygonizeVDBO(VDBO* vdbo) {
	vdbo->polygonizeUniformly();
}

VDBLIB_API void PolygonizeVDBOAdaptive(VDBO* vdbo, float iso, float adaptivity) {
	vdbo->polygonizeAdaptively(iso, adaptivity);
}

VDBLIB_API float* GetVertexBuffer(VDBO* vdbo, int* size) {
	
	int sz = vdbo->pointsLen();
	*size = 3 * sz; //out1
	
	float* vertexarray = reinterpret_cast<float*>(malloc(sizeof(float) * (*size)));// = vdbo->mPoints; 

	int i = 0;
	for (auto iter = vdbo->mPoints.begin(); iter != vdbo->mPoints.end(); iter++) {
		openvdb::Vec3s p = *iter;
		vertexarray[i + 0] = p.x();
		vertexarray[i + 1] = p.y();
		vertexarray[i + 2] = p.z();
		i = i + 3;
	}

	return vertexarray; //out2
}

VDBLIB_API int GetVertexCount(VDBO* vdbo) {
	return vdbo->pointsLen();
}
/*
extern VDBLIB_API int* GetQuadIndices(VDBO* vdbo, int* size) {
	
	int sz = vdbo->quadsLen();
	*size = 4 * sz;//out1

	int* quadindices = reinterpret_cast<int*>(malloc(sizeof(int) * (*size)));// = vdbo->mQuads; 

	int i = 0;
	for (auto iter = vdbo->mQuads.begin(); iter != vdbo->mQuads.end(); iter++) {
		openvdb::Vec4I t = *iter;
		quadindices[i + 0] = t.w();
		quadindices[i + 1] = t.x();
		quadindices[i + 2] = t.y();
		quadindices[i + 3] = t.z();
		i = i + 4;
	}

	return quadindices; //out2
}

VDBLIB_API int GetQuadCount(VDBO* vdbo) {
	return vdbo->quadsLen();
}
*/


VDBLIB_API int* GetTriangleIndices(VDBO* vdbo, int* size) {

	int sz = vdbo->trianglesLen();
	*size = 3 * sz;//out1

	int* triangleindices = reinterpret_cast<int*>(malloc(sizeof(int) * (*size)));// = vdbo->mTriangles; 

	int i = 0;
	for (auto iter = vdbo->mTriangles.begin(); iter != vdbo->mTriangles.end(); iter++) {
		openvdb::Vec3I t = *iter;
		triangleindices[i + 0] = t.x();
		triangleindices[i + 1] = t.y();
		triangleindices[i + 2] = t.z();
		i = i + 3;
	}

	return triangleindices; //out2
}

VDBLIB_API int GetTriangleCount(VDBO* vdbo) {
	return vdbo->trianglesLen();
}



VDBLIB_API float GetVoxelSize(VDBO* vdbo)
{
	if (vdbo != nullptr) {
		return vdbo->mGrid->voxelSize().x();//TODO:assumption x=y=z
	}
	return -1.0f;
}

VDBLIB_API void ErodeVDBO(VDBO* vdbo, int iters) {
	vdbo->erodeVolume(iters);
}

VDBLIB_API VDBO* CsgDifferenceVDBO(VDBO* vdboA, VDBO* vdboB){
	//TODO transforms, also on VDBO side
	vdboA->csgDifferenceInplace(vdboB);
	return vdboA;
}

VDBLIB_API bool WriteVDBFile(VDBO* vdbo, const char* file)
{
	return vdbo->writeToVDBFile(file);
}

VDBLIB_API void TranslateVDBO(VDBO* vdbo, float x, float y, float z)
{
	vdbo->translate(x, y, z);
	return;
}

VDBLIB_API void SetPositionVDBO(VDBO* vdbo, float x, float y, float z)
{
	vdbo->setPosition(x, y, z);
	return;
}

VDBLIB_API void DestroyVDBO(VDBO *vdbo) {
	if (vdbo != NULL) {

		/*std::cout << "___VDBO Deleted:___" << std::endl;
		std::cout << vdbo->mInfo << std::endl;
		std::cout << "XXXXXXXXXXXXXXXXXXX" << std::endl;*/

		delete vdbo;
		vdbo = NULL;
	}
}