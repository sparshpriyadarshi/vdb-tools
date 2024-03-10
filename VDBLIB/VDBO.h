#pragma once

#include <openvdb/openvdb.h>
#include <openvdb/tools/LevelSetSphere.h>
#include <openvdb/tools/LevelSetPlatonic.h>
#include <openvdb/tools/ChangeBackground.h>
#include <openvdb/tools/Morphology.h>
#include <openvdb/tools/VolumeToMesh.h>
#include <openvdb/tools/Composite.h>
#include <openvdb/tools/Clip.h>

#include <openvdb/points/PointConversion.h>
#include <openvdb/points/PointCount.h>
#include <openvdb/math/BBox.h>

#include <iostream>
#include <fstream>
#define _USE_MATH_DEFINES // for C++
#include <cmath>

class VDBO
{
public:
	std::string mInfo;
	std::string mFileSource;
	openvdb::FloatGrid::Ptr mGrid;
	std::vector<openvdb::Vec3s> mPoints;
	std::vector<openvdb::Vec4I> mQuads;
	std::vector<openvdb::Vec3I> mTriangles;

	// creates empty grid TODO revies impl
	VDBO();
	// init grid with sample cube
	void initSampleCube(float voxelSize, float dim, float posx, float posy, float posz);
	// creates sample sphere replaces the cube TODO refactor to rename this method, update refs (initSampleSphere)
	void changeSampleToSphere();
	// reads VDB file from path
	VDBO(std::string filepath);

	~VDBO();

	void polygonizeUniformly();
	void polygonizeAdaptively(float isoval, float adaptivity);
	void clearPolygonsUtil();
	
	int pointsLen();
	int quadsLen();
	int trianglesLen();
	
	// create vdb volume out of polygons
	void voxelizePolygons(float voxelSize, float bandwidth);

	void makeRandomVolumeChange();
	void erodeVolume(int iters);
	
	// intersection of this volume with the other and subtract the overlap.
	void csgDifferenceInplace(VDBO* otherVDBO);
	void getBoundingBox();

	void translate(float x, float y, float z);

	void setPosition(float x, float y, float z);

	//write to filepath
	bool writeToVDBFile(std::string fpath);       
private:
	std::stringstream _info;

	int _psize;

	int _qsize;
	int _tsize;

	//internal operation for quads polygonization
	void polygonizeToQuadsUniformly();

	//internal operation for triangle polygonization
	void polygonizeToTrianglesUniformly();

};