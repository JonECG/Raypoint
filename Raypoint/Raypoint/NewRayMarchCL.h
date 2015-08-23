#pragma once

#include "Camera.h"
#include <CL/cl.h>

class NewRayMarchCL
{
	bool rewriteSubs;
	bool programAlreadyLoaded;

	static const int sized = 128, MWIDTH = 2000, MHEIGHT = 2000, MDATUMS = 20;

	glm::vec4 *resultSet;
	glm::vec4 position;
	
	glm::mat4 inverseMVMatrix, inverseProjMatrix;

	cl_mem inInvMVMatrix, inInvProjMatrix, inSubdividePositions, inData, outResult;

	cl_context context;
	cl_command_queue queue;
	cl_kernel rayKernel;

	cl_program program;
	cl_device_id device;

	void init();
public:
	bool runningThread;
	bool quitFlag;

	Camera * camera;

	glm::vec2 *previousDimensions;
	glm::vec2 *subdividePositions, *subdivideSizes;
	
	int calculatedAmount;

	NewRayMarchCL();
	
	void initialize();
	bool launch( glm::vec2 dimensions );
	void load( const char * filename );
	void unload();
	void destroy();

	void stopWait();

	glm::vec4 getPixel( int x, int y );
	glm::vec4 getPixel( int index );

	//void setCamera(Camera& cam);
	//Camera getCamera();

	void setData( int index, float data );
	void setData( int index, float * data, int count );

	bool isComplete();
};

