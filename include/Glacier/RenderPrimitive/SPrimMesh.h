#pragma once

#include "SPrimObject.h"
#include "../Math/SVector4.h"
#include "../Math/SVector2.h"

struct SPrimMesh : SPrimObject
{
	unsigned int lSubMeshTable;
	unsigned int lNumFrames;
	unsigned short lFrameStart;
	unsigned short lFrameStep;
	SVector4 vPosScale;
	SVector4 vPosBias;
	SVector2 vTexScale;
	SVector2 vTexBias;
};
