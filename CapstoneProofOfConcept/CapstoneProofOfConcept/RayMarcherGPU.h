#pragma once

#include "Camera.h"
#include <CL/cl.h>
#include "QtGui\qwidget.h"

class RayMarcherGPU
{
	Camera * camera;
	const char * fileName;

	static const int sized = 128, MWIDTH = 512, MHEIGHT = 512;

	glm::vec4 *resultSet;
	glm::vec4 position;
	
	glm::mat4 inverseMVMatrix, inverseProjMatrix;

	cl_mem inInvMVMatrix, inInvProjMatrix, inSubdividePositions, outResult;// inDimensions, inPosition, ;

	cl_context context;
	cl_command_queue queue;
	cl_kernel vector_add_kernel;

	cl_program program;
	cl_device_id device;

	void init();

	QWidget * widg;
	
public:
	glm::vec2 *previousDimensions;
	glm::vec2 *subdividePositions, *subdivideSizes;
	

	RayMarcherGPU( const char * fileName, Camera * camera, QWidget * widg );
	glm::vec4 getPixel( int x, int y );
	glm::vec4 getPixel( int index );
	void initialize();
	void launch();
	void load();
	void unload();
	void destroy();
};

