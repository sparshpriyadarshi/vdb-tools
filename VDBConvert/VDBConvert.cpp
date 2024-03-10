// VDBConvert.cpp : This file contains the 'main' function. Program execution begins and ends there.
// Commandline utility to convert models to VDB files, inspired from OpenVDBCompiler in delta3d

#pragma once
#include <openvdb/openvdb.h>

#include <iostream>
#include <fstream>

#define _USE_MATH_DEFINES // for C++

#include <cmath>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgViewer/Viewer>
#include <osg/ShapeDrawable>
#include <osg/Geode>
#include <osg/Geometry>
#include <osgUtil/SmoothingVisitor>
#include <osg/Group>
#include <osg/PolygonMode>
//functors
#include <osg/TemplatePrimitiveFunctor>
#include <osg/TemplatePrimitiveIndexFunctor>
#include "VDBLibrary.h"
#include <filesystem>

#include <tbb/parallel_for.h>



struct TriangleCollector
{
	std::vector<osg::Vec3f> mPts; //TODO check if needs to be double
	std::vector<osg::Vec3i> mInds;
	void operator() (const osg::Vec3& v1, bool) const
	{
		//std::cout << "\rpoint(" << v1.x() << "," << v1.y() << "," << v1.z() << ")" << std::endl;
	}

	void operator() (const osg::Vec3& v1, const osg::Vec3& v2, bool) const
	{
		//std::cout << "\tline(" << v1.x() << "," << v1.y() << "," << v1.z() << ") (" << v2.x() << "," << v2.y() << "," << v2.z() << ")" << std::endl;
	}

	void operator() (const osg::Vec3& v1, const osg::Vec3& v2, const osg::Vec3& v3, bool)
	{
		//osg::Vec3 normal = (v2 - v1) ^ (v3 - v2);
		//normal.normalize();
		//std::cout << "\ttriangle(" << v1.x() << "," << v1.y() << "," << v1.z() << ") (" << v2.x() << "," << v2.y() << "," << v2.z() << ") (" << v3.x() << "," << v3.y() << "," << v3.z() << ") " << ") normal (" << normal.x() << "," << normal.y() << "," << normal.z() << "," << ")" << std::endl;
		mPts.push_back(osg::Vec3f(v1.x(), v1.y(), v1.z()));
		mPts.push_back(osg::Vec3f(v2.x(), v2.y(), v2.z()));
		mPts.push_back(osg::Vec3f(v3.x(), v3.y(), v3.z()));
		int faceIdxBase = mPts.size() - 3;
		mInds.push_back(osg::Vec3i(faceIdxBase, faceIdxBase + 1, faceIdxBase + 2));
	}

	void operator() (const osg::Vec3& v1, const osg::Vec3& v2, const osg::Vec3& v3, const osg::Vec3& v4, bool) const
	{
		//osg::Vec3 normal = (v2 - v1) ^ (v3 - v2);
		//normal.normalize();
		//std::cout << "\tquad(" << v1.x() << "," << v1.y() << "," << v1.z() << ") (" << v2.x() << "," << v2.y() << "," << v2.z() << ") (" << v3.x() << "," << v3.y() << "," << v3.z() << ") (" << v4.x() << "," << v4.y() << "," << v4.z() << ") " << ")" << std::endl;
	}
};

bool parseInput(int argc, char* argv[], bool* floatTypeFlag, float* vxlSize, float* vxlSizeZ, float* bw,  std::string* fn) {
	bool returnCode = true;

	try {
		std::cout << "argc = " << argc << std::endl;
		for (int i = 0; i < argc; i++) {
			std::string currentArg = argv[i];
			std::string nextArg;
			if (i + 1 < argc)  nextArg = argv[i + 1];
	
			std::cout << "option: " << currentArg << std::endl;
			if (i == 0) continue;
			if (currentArg.substr(0,2) == "-f") {
				*floatTypeFlag = true;
			}
			else if (currentArg.substr(0, 2) == "-b") {
				*floatTypeFlag = false;
			}
			else if (currentArg.substr(0, 2) == "-r" && std::stof(nextArg) >= 0.0 && std::stof(nextArg) <= 1.0) {
				*vxlSize = std::stof(nextArg);
				*vxlSizeZ = *vxlSize;
				i++;
			}
			else if (currentArg.substr(0, 2) == "-z" && std::stof(nextArg) >= 0.0 && std::stof(nextArg) <= 1.0) {
				*vxlSizeZ = std::stof(nextArg);
				i++;
			}
			else if (currentArg.substr(0, 2) == "-t" && std::stof(nextArg) >= 0.0 && std::stof(nextArg) <= 10.0) { //custom limits might change later
				*bw = std::stof(nextArg);
				i++;
			}
			else {
				fn->append(currentArg);
			}
		}
	}
	catch (std::exception &e) {
		returnCode = false;
	}
	
	return returnCode;
}

int main(int argc, char* argv[], char* envp[])
{
	// sensible defaults
	bool floatTypeFlag = true; // unless -b is passed
	float vxlSize = 1.0F;		   // for x,y,z
	float vxlSizeZ = vxlSize;      // default untill modified
	float bandwidth = 1.0F;
	
	std::string fn; //input filename
	std::string ofn; //output filename

	if (!parseInput(argc, argv, &floatTypeFlag, &vxlSize, &vxlSizeZ, &bandwidth, &fn)) {
		std::cerr << "Input args parse failed, exiting..." << std::endl;
		return -1;
	}


	/*std::cout << "Input parse complete..." << std::endl;
	std::cout << "floatTypeFlag " << floatTypeFlag << std::endl;
	std::cout << "vxlSize " << vxlSize << std::endl;
	std::cout << "vxlSizeZ " << vxlSizeZ << std::endl;
	std::cout << "bw " << bandwidth << std::endl;
	std::cout << "fn " << fn << std::endl;
	std::cout << "Input parse complete..." << std::endl;*/

	//std::filesystem::path ifp(fn), ofp(ofn);
	
	//std::filesystem::path ofp(ofn);
	//std::cout << ofp << " is being writtern" << std::endl;
	//std::cout << fp << std::endl;
	//std::cout << *(fp.c_str()) << std::endl;
	//std::cout << fp.generic_string() << std::endl;
	

	osg::ref_ptr<osg::Node> objNode = osgDB::readNodeFile(fn);
	osg::ref_ptr<osg::Geode> geode = (objNode->asGroup()->getChild(0))->asGeode(); //ASSUMPTIONS
	osg::ref_ptr<osg::Geometry> geom = geode->getDrawable(0)->asGeometry();

	osg::TemplatePrimitiveFunctor<TriangleCollector> tf;
	geom->accept(tf);

	float* vArr = new float[tf.mPts.size() * 3];
	int* tArr = new int[tf.mInds.size() * 3];


	/*for (int i = 0, offset = 0; i < tf.mPts.size(); i++, offset += 3) {
		vArr[offset + 0] = tf.mPts[i].x();
		vArr[offset + 1] = tf.mPts[i].y();
		vArr[offset + 2] = tf.mPts[i].z();
	}*/
	tbb::parallel_for(tbb::blocked_range<int>(0, tf.mPts.size()), 
		[&](tbb::blocked_range<int> r) {
		for (int i = r.begin(); i < r.end(); ++i) {
			int offset = i * 3;
			vArr[offset + 0] = tf.mPts[i].x();
			vArr[offset + 1] = tf.mPts[i].y();
			vArr[offset + 2] = tf.mPts[i].z();
		}
	});
	
	/*for (int i = 0, offset = 0; i < tf.mInds.size(); i++, offset += 3) {
		tArr[offset + 0] = tf.mInds[i].x();
		tArr[offset + 1] = tf.mInds[i].y();
		tArr[offset + 2] = tf.mInds[i].z();
	}*/
	tbb::parallel_for(tbb::blocked_range<int>(0, tf.mInds.size()),
		[&](tbb::blocked_range<int> r) {
			for (int i = r.begin(); i < r.end(); ++i) {
				int offset = i * 3;
				tArr[offset + 0] = tf.mInds[i].x();
				tArr[offset + 1] = tf.mInds[i].y();
				tArr[offset + 2] = tf.mInds[i].z();
			}
		});

	VDBO* vdbo = CreateSampleVDBO();
	
	vdbo = CreateVDBOFromMesh(
		vdbo, 
		vArr, 
		tf.mPts.size() * 3, 
		tArr, 
		tf.mInds.size() * 3, 
		vxlSize, 
		bandwidth // number of voxel units
	);

	std::cout << GetVDBOInfo(vdbo) << std::endl;
	ofn = fn.append(".vdb"); // modifies fn
	boolean result = WriteVDBFile(vdbo, ofn.c_str());

	if (result) std::cout << "SUCCESS" << std::endl;
	else std::cout << "ERROR" << std::endl;

	//CLEANUP
	DestroyVDBO(vdbo);
	//
		
	return result ? 0 : -1;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
/*
option: D:\development\vdb-dev\ConsoleVDBSample\x64\Release\VDBConvert.exe
option: -f
option: -r
option: 0.15
option: -z
option: 0.05
option: ./assets/teapot.obj



*/