#include "pch.h"
#include "VDBO.h"


VDBO::VDBO() {
	openvdb::initialize();
}
VDBO::VDBO(std::string filepath){
	VDBO();
	
	openvdb::io::File file = openvdb::io::File(filepath);
	openvdb::GridBase::Ptr baseGrid;

	file.open();
	for (openvdb::io::File::NameIterator nameIter = file.beginName(); nameIter != file.endName(); ++nameIter)
	{
		baseGrid = file.readGrid(nameIter.gridName());
		break;
	}
	mGrid = openvdb::gridPtrCast<openvdb::FloatGrid>(baseGrid);

	mGrid->print(_info);
	mInfo = _info.str();

	file.close();
}
void VDBO::initSampleCube(float voxelSize, float dim, float posx, float posy, float posz){
	
	mGrid = openvdb::tools::createLevelSetCube<openvdb::FloatGrid>(
		dim,
		openvdb::Vec3f(posx,posy,posz), 
		voxelSize, 
		static_cast<float>(openvdb::LEVEL_SET_HALF_WIDTH)
	);


	mGrid->insertMeta("scale", openvdb::FloatMetadata(10.0));

	//mGrid->setTransform(openvdb::math::Transform::createLinearTransform(voxelSize));
	// Identify the grid as a level set.
	mGrid->setGridClass(openvdb::GRID_LEVEL_SET);
	// Name the grid
	mGrid->setName("LevelSetCube");

	_info.clear();
	mGrid->print(_info);
	mInfo = _info.str();
}
void VDBO::changeSampleToSphere() {
	
	mGrid =	openvdb::tools::createLevelSetSphere<openvdb::FloatGrid>(
			/*radius=*/10.0, /*center=*/openvdb::Vec3f(0, 0, 0),
			/*voxel size=*/0.1, /*width=*/1.0);
	mGrid->insertMeta("radius", openvdb::FloatMetadata(10.0));
	// Associate a scaling transform with the grid that sets the voxel size
	// to 1.0 units in world space.
	mGrid->setTransform(openvdb::math::Transform::createLinearTransform(1.0));//TODO check
	mGrid->setGridClass(openvdb::GRID_LEVEL_SET);	
	mGrid->setName("LevelSetSphere");

	//update info on the grid...
	_info.clear();
	mGrid->print(_info);
	mInfo = _info.str();

	// Repolygonize important.
	//update: what if we don't in the API...
	//polygonizeToTrianglesUniformly();
}

void VDBO::makeRandomVolumeChange() {
	// 5 is a random value
	openvdb::tools::erodeActiveValues<openvdb::FloatTree>(mGrid->tree(), 5);
}
void VDBO::erodeVolume(int iters) {
	openvdb::tools::erodeActiveValues<openvdb::FloatTree>(mGrid->tree(), iters);
}

void VDBO::polygonizeToQuadsUniformly() {
	mPoints.clear(); mQuads.clear(); mTriangles.clear();
	_psize = 0; _qsize = 0; _tsize = 0;

	openvdb::tools::volumeToMesh<openvdb::FloatGrid>(*mGrid, mPoints, mQuads, 0.0); //uniform at the moment
	
	_psize = mPoints.size();
	_qsize = mQuads.size();
	_tsize = mTriangles.size(); 
}

void VDBO::polygonizeToTrianglesUniformly() {
	VDBO::polygonizeToQuadsUniformly();
	for (auto iter = mQuads.begin(); iter != mQuads.end(); iter++) {
		openvdb::Vec4I q = *iter;
		//openvdb::Vec3I t1(q[3], q[2], q[1]);
		//openvdb::Vec3I t2(q[1], q[0], q[3]);
		openvdb::Vec3I t1(q[1], q[2], q[3]);
		openvdb::Vec3I t2(q[3], q[0], q[1]);
		mTriangles.push_back(t1);
		mTriangles.push_back(t2);
	}
	_tsize = mTriangles.size();

	mQuads.clear();
	_qsize = 0;

}


void VDBO::polygonizeUniformly() {
	VDBO::polygonizeToTrianglesUniformly();
}
void VDBO::polygonizeAdaptively(float iso, float adaptivity) {
	if (adaptivity < 0.0 || adaptivity > 1.0) {
		//clip it to a default.
		adaptivity = 0.5;
	}

	mPoints.clear(); mQuads.clear(); mTriangles.clear();
	_psize = 0; _qsize = 0; _tsize = 0;

	openvdb::tools::volumeToMesh<openvdb::FloatGrid>(*mGrid, mPoints, mTriangles, mQuads, iso, adaptivity);

	for (auto iter = mQuads.begin(); iter != mQuads.end(); iter++) {
		openvdb::Vec4I q = *iter;
		//openvdb::Vec3I t1(q[3], q[2], q[1]);
		//openvdb::Vec3I t2(q[1], q[0], q[3]);
		openvdb::Vec3I t1(q[1], q[2], q[3]);
		openvdb::Vec3I t2(q[3], q[0], q[1]);
		mTriangles.push_back(t1);
		mTriangles.push_back(t2);
	}
	_psize = mPoints.size();
	_tsize = mTriangles.size();
	_qsize = mQuads.size();


	mQuads.clear();
	_qsize = 0;
}
void VDBO::clearPolygonsUtil(){
	mPoints.clear();
	mQuads.clear();
	mTriangles.clear();
}

void VDBO::csgDifferenceInplace(VDBO* otherVDBO){
	
	//auto copy = otherVDBO->mGrid->deepCopy();
	
	mGrid = openvdb::tools::csgDifferenceCopy(*mGrid, *otherVDBO->mGrid);
	
	// clumsy reset, not working right now
	otherVDBO->initSampleCube(mGrid->voxelSize().x(),1.0f, 0.0f,0.0f,0.0f);

	//otherVDBO->mGrid = copy;

}

void VDBO::getBoundingBox() {
	
}

void VDBO::translate(float x, float y, float z){
	mGrid->transform().postTranslate(openvdb::Vec3f(x, y, z));
}

void VDBO::setPosition(float x, float y, float z)
{
	mGrid->setTransform(openvdb::math::Transform::createLinearTransform(mGrid->transform().voxelSize().x()));//TODO: Assumption x=y=z
	mGrid->transform().postTranslate(openvdb::Vec3f(x, y, z));
}

void VDBO::voxelizePolygons(float voxelSize, float bandwidth) {
	//faces and triangles should be already here.

	for (int pi = 0; pi < mPoints.size(); pi++) {
		mPoints[pi] *= (static_cast<float>(1.0/voxelSize)); 
	}
	openvdb::math::Transform xform;
	xform.preScale(voxelSize);

	//auto xform = openvdb::math::Transform::createLinearTransform(voxelSize);
	//mGrid = openvdb::tools::meshToLevelSet<openvdb::FloatGrid>(*xform, mPoints, mTriangles, static_cast<float>(bandwidth));
	
	//mGrid = openvdb::tools::meshToLevelSet<openvdb::FloatGrid>(xform, mPoints, mTriangles, static_cast<float>(bandwidth));
	//TODO: think about the role of scale here, is it correct for this application ?
	openvdb::tools::QuadAndTriangleDataAdapter<openvdb::Vec3s, openvdb::Vec3I> polygonAdapter(mPoints, mTriangles);
	mGrid = openvdb::tools::meshToVolume<openvdb::FloatGrid>(polygonAdapter, xform, static_cast<float>(bandwidth), static_cast<float>(bandwidth), 0, nullptr);

	mGrid->setName("Voxelized Geometry ");
	mGrid->insertMeta("scale", openvdb::FloatMetadata(voxelSize)); 
	mGrid->setGridClass(openvdb::GRID_LEVEL_SET);
	mGrid->setSaveFloatAsHalf(true);
	_info.clear();
	mGrid->print(_info);
	mInfo = _info.str();

}

bool VDBO::writeToVDBFile(std::string fpath) {
	
	openvdb::io::File wfile(fpath);
	//TODO optional: metadata write
	openvdb::GridPtrVec grids;
	grids.push_back(mGrid);
	wfile.write(grids);
	
	wfile.close();
}

int VDBO::pointsLen() {
	return _psize;
}
int VDBO::quadsLen() {
	return _qsize;
}
int VDBO::trianglesLen() {
	return _tsize;
}

VDBO::~VDBO() {
	/*if (mFile.isOpen()) {
		mFile.close();
	}*/
}