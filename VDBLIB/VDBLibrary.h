#pragma once
// <PROJECTNAME_EXPORTS> defined by MSVStudio...
#ifdef VDBLIB_EXPORTS
#define VDBLIB_API __declspec(dllexport)
#else
#define VDBLIB_API __declspec(dllimport)
#endif

#include "VDBO.h"


#ifdef __cplusplus
extern "C" {
#endif


	/* VDB Object Interface */

	// create empty VDB Object
	extern VDBLIB_API VDBO* CreateEmptyVDBO();
	// initialize with simple VDB cube object.
	extern VDBLIB_API VDBO* CreateSampleVDBO();
	extern VDBLIB_API VDBO* CreateSampleVDBOCustom(float vxlSize, float len, float posx, float posy, float posz);

	// change cube to sphere. (client polygonize after this IMP)
	extern VDBLIB_API VDBO* ChangeToSphereVDBO(VDBO* vdbo);
	// take VDB filepath and initialize VDB object, Error checks not done so far, client is advised to make sure path is valid and accesible.
	extern VDBLIB_API VDBO* CreateVDBO(const char* filepath);
	// initialize VDB from mesh triangles. Currently, client should have a sample vdbo initialized and pass it here
	extern VDBLIB_API VDBO* CreateVDBOFromMesh(VDBO* vdbo, float *verts, int vCount, int *faces, int fCount, double voxelSize, double bandwidth);
	// write to a VDB file, client ensure file path is well behaved, currently expects escapes to be escaped.
	extern VDBLIB_API bool WriteVDBFile(VDBO* vdbo, const char* file);//TODO

	// get human-readable info for vdbo created.
	extern VDBLIB_API char* GetVDBOInfo(VDBO* vdbo);
	extern VDBLIB_API char* VDBLibErrorInfo();

	// polygonize the VDB Object.
	extern VDBLIB_API void PolygonizeVDBO(VDBO* vdbo);
	// polygonize the VDB Object adaptively; iso default 0.0, adaptivity 0.5
	extern VDBLIB_API void PolygonizeVDBOAdaptive(VDBO* vdbo, float iso, float adaptivity);

	// get vertices
	extern VDBLIB_API float* GetVertexBuffer(VDBO* vdbo, int* size);
	// get vertex count
	extern VDBLIB_API int GetVertexCount(VDBO* vdbo);

	// get triangles
	extern VDBLIB_API int* GetTriangleIndices(VDBO* vdbo, int* size);
	// get Triangle count
	extern VDBLIB_API int GetTriangleCount(VDBO* vdbo);
 	// get quads
	//extern VDBLIB_API int* GetQuadIndices(VDBO* vdbo, int* size);
	// get Quad count
	//extern VDBLIB_API int GetQuadCount(VDBO* vdb0); 

	// get voxel size from grid
	extern VDBLIB_API float GetVoxelSize(VDBO* vdbo);
	
	// erode active values in vdbo tree, (vertex buffer not affected)
	extern VDBLIB_API void ErodeVDBO(VDBO* vdbo, int iters);

	// perform csg difference with another vdbo, first vdbo overwrittern
	extern VDBLIB_API VDBO* CsgDifferenceVDBO(VDBO* vdboA, VDBO* vdboB);
	//TODO union
	//TODO intersection
	//TODO other csg ops


	extern VDBLIB_API void TranslateVDBO(VDBO* vdbo, float x, float y, float z);
	
	//reset to zero and translate (transform)
	extern VDBLIB_API void SetPositionVDBO(VDBO* vdbo, float x, float y, float z);

	// release memory for VDB initialized.
	extern VDBLIB_API void DestroyVDBO(VDBO* vdbo);



#ifdef __cplusplus
}
#endif

