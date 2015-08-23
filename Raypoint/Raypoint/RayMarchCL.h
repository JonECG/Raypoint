#pragma once

#include "Camera.h"
#include <CL/cl.h>



class RayMarchCL
{
	Camera * camera;
	const char * fileName;

	//std::thread * currentThread

	bool rewriteSubs;

	static const int sized = 128, MWIDTH = 1920, MHEIGHT = 1080;

	glm::vec4 *resultSet;
	glm::vec4 position;
	
	glm::mat4 inverseMVMatrix, inverseProjMatrix;

	cl_mem inInvMVMatrix, inInvProjMatrix, inSubdividePositions, outResult;// inDimensions, inPosition, ;

	cl_context context;
	cl_command_queue queue;
	cl_kernel rayKernel;

	cl_program program;
	cl_device_id device;

	

	void init();
	
public:
	bool runningThread;
	bool quitFlag;

	glm::vec2 *previousDimensions;
	glm::vec2 *subdividePositions, *subdivideSizes;
	
	int calculatedAmount;

	RayMarchCL( const char * fileName, Camera * camera );
	glm::vec4 getPixel( int x, int y );
	glm::vec4 getPixel( int index );
	void initialize();
	bool launch( glm::vec2 dimensions );
	void load();
	void unload();
	void destroy();
};

