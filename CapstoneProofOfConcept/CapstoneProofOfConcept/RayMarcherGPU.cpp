#include "RayMarcherGPU.h"

#include <iostream>
#include <fstream>

RayMarcherGPU::RayMarcherGPU( const char * fileName, Camera * camera, QWidget * widg )
{
	this->fileName = fileName;
	this->camera = camera;
	this->widg = widg;
}

void printError( const char * location, cl_int error )
{
	if (error != CL_SUCCESS) 
	{
	   std::cout << "ERROR \"" << location << "\": " << error << std::endl;
	   //exit(error);
	}
}

std::size_t filesize(const char* filename)
{
	//std::ifstream in(filename, std::ios::ate || std::ios::binary);
	//std::size_t result = in.tellg();

	std::ifstream in(filename, std::ifstream::in | std::ifstream::binary);
    in.seekg(0, std::ifstream::end);
    std::size_t result = in.tellg(); 

	in.close();
    return result; 
}

char* fileContents( const char* filename, std::size_t * fileSize )
{
	*fileSize = filesize( filename ) + 1;
	char * result = new char[ *fileSize ];
	result[ *fileSize - 1 ] = '\0';

	std::ifstream in(filename, std::ios::binary);
	in.read(result, *fileSize - 1);
	in.close();

	return result;
}

int multipleRoundUp( int multiple, int target )
{
	return multiple * (target/multiple + 1);
}

void RayMarcherGPU::destroy()
{
	// Cleaning up
	delete[] resultSet;
	delete[] subdivideSizes;
	delete[] subdividePositions;

	clReleaseCommandQueue(queue);
	clReleaseContext(context);

	unload();
}

void RayMarcherGPU::init()
{
	//Allocate Memory
	/*inverseMVMatrix = camera->getInvModelView();
	inverseProjMatrix = camera->getInvProjection();*/
	//resultSet = new glm::vec4[MWIDTH*MHEIGHT];
	//subdividePositions = new glm::vec2[MWIDTH*MHEIGHT];
	//subdivideSizes = new glm::vec2[MWIDTH*MHEIGHT];
	previousDimensions = new glm::vec2( 0 );
	position = glm::vec4( camera->from, 0 );
}

void RayMarcherGPU::load()
{
	cl_int error = 0;
	int size = sized;
	/*
	clCreateBuffer flags
	--------------------
	CL_MEM_READ_WRITE
	CL_MEM_WRITE_ONLY
	CL_MEM_READ_ONLY
	CL_MEM_USE_HOST_PTR
	CL_MEM_ALLOC_HOST_PTR
	CL_MEM_COPY_HOST_PTR - copies the memory pointed by host_ptr
	*/

	//Create Buffers
	inInvProjMatrix = clCreateBuffer( context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof( glm::mat4 ), &inverseProjMatrix, &error );
	inInvMVMatrix = clCreateBuffer( context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof( glm::mat4 ), &inverseMVMatrix, &error );
	inSubdividePositions = clCreateBuffer( context, CL_MEM_READ_ONLY, sizeof( glm::vec2 ) * MWIDTH*MHEIGHT, 0, &error );
	outResult = clCreateBuffer( context, CL_MEM_WRITE_ONLY, sizeof( glm::vec4 ) * MWIDTH*MHEIGHT, 0, &error );
	//inDimensions = clCreateBuffer( context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof( glm::vec2 ), &dimensions, &error );
	//inPosition = clCreateBuffer( context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof( glm::vec4 ), &position, &error );

	
	std::cout << "Compiling" << std::endl;
	// Creates the program
	size_t src_size = 0;

#ifdef _DEBUGGY
	const char * source = fileContents( "ray_march.cl", &src_size );
#else
	const char * source = fileContents( "../ray_march.cl", &src_size );
#endif
	
	program = clCreateProgramWithSource(context, 1, &source, &src_size, &error);
	//std::cout << source << std::endl;
	
	delete source; //TODO: FIGURE OUT WHY THE FUCKING ASSWAFFLE THIS ISN'T WORK

	printError( "Creating Program", error );
	
	// Builds the program
	error = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
	printError( "Building Program", error );


	// Extracting the kernel
	vector_add_kernel = clCreateKernel(program, "ray_march_gpu", &error);
	printError( "Extract Kernel", error );

	//Set Up Arguments
	// Note that we inform the size of the cl_mem object, not the size of the memory pointed by it
	error = clSetKernelArg(vector_add_kernel, 0, sizeof(cl_mem), &inInvProjMatrix);
	error |= clSetKernelArg(vector_add_kernel, 1, sizeof(cl_mem), &inInvMVMatrix);
	error |= clSetKernelArg(vector_add_kernel, 2, sizeof(cl_mem), &inSubdividePositions);
	error |= clSetKernelArg(vector_add_kernel, 3, sizeof(cl_mem), &outResult);
	printError( "Set up arguments", error );

	// Shows the log
	char* build_log;
	size_t log_size;
	// First call to know the proper size
	clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
	build_log = new char[log_size+1];
	// Second call to get the log
	clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_size, build_log, NULL);
	build_log[log_size] = '\0';
	std::cout << build_log << std::endl;
	delete[] build_log;
}

void RayMarcherGPU::unload()
{
	clReleaseProgram(program);
	clReleaseKernel(vector_add_kernel);
	clReleaseMemObject(inInvMVMatrix);
	clReleaseMemObject(inInvProjMatrix);
	clReleaseMemObject(inSubdividePositions);
	clReleaseMemObject(outResult);
}

void RayMarcherGPU::launch()
{
	cl_int error = 0;
	int size = sized;

	//clFinish( queue );

	inverseMVMatrix = camera->getInvModelView();
	inverseProjMatrix = camera->getInvProjection();
	position = glm::vec4( camera->from, 0 );
	//glm::vec2 oldDim = *previousDimensions;
	glm::vec2 newDimensions = glm::vec2( widg->width(), widg->height() );

	//std::cout << "Comp " << previousDimensions->x << " " << newDimensions.x << std::endl;

	if( oldDim.x != newDimensions.x || oldDim.y != newDimensions.y )
	{
		//std::cout << "Dis " << previousDimensions->x << " " << newDimensions.x << std::endl;
		previousDimensions->x = newDimensions.x;
		previousDimensions->y = newDimensions.y;
		int index = 0;

		subdividePositions[index] = glm::vec2( 0 );
		subdivideSizes[index] = glm::vec2(1);

		index++;

		//bool * hitX = new bool[(int)newDimensions.x];
		//bool * hitY = new bool[(int)newDimensions.y];

		bool * hit = new bool[(int)(newDimensions.x*newDimensions.y)];

		for( int i = 0; i < newDimensions.x*newDimensions.y; i++ )
		{
			hit[i] = false;
		}
		hit[0] = true;

		//hitX[0] = true;
		//hitY[0] = true;

		//TODO, just pull highest order bit
		for( int iteration = 0; iteration < std::log( std::min( newDimensions.x,newDimensions.y ) )/ std::log(2)- 1; iteration++ )
		{
			int numGroups = std::pow( 2, iteration );

			for( int xGroup = 0; xGroup < numGroups; xGroup ++ )
			{
				for( int yGroup = 0; yGroup < numGroups; yGroup ++ )
				{
					for( int cell = 1; cell < 4; cell++ )
					{
						float groupAmount = 1.0f / numGroups;

						//float x = ( xGroup + ( (cell%2!=0)? 0.5 : 0 ) ) * groupAmount;
						//float y = ( yGroup + ( (cell>1)? 0.5 : 0 ) ) * groupAmount;

						/*int imageX = x * image->width();
						int imageY = y * image->height();*/

						subdividePositions[index] = glm::vec2( xGroup + ( (cell%2!=0)? 0.5 : 0 ), yGroup + ( (cell>1)? 0.5 : 0 ) ) * groupAmount;
						/*hitX[(int)(subdividePositions[index].x*newDimensions.x)]=true;
						hitY[(int)(subdividePositions[index].y*newDimensions.y)]=true;*/
						int x = subdividePositions[index].x*newDimensions.x;
						int y = subdividePositions[index].y*newDimensions.y;
						hit[(int)(subdividePositions[index].x*newDimensions.x + subdividePositions[index].y*newDimensions.x*newDimensions.y )]=true;
						subdivideSizes[index] = glm::vec2( groupAmount ) / 2.0f;
						index++;
					}
				}	
			}
		}

		for( int x = 0; x < newDimensions.x; x++ )
		{
			for( int y = 0; y < newDimensions.y; y++ )
			{
				//if( !hitX[x] || !hitY[y] 
				if( !hit[(int)(x + y*newDimensions.x )] )
				{
					subdividePositions[index] = glm::vec2( x/newDimensions.x, y/newDimensions.y );
					subdivideSizes[index] = glm::vec2( 0.00001 );
					index++;
				}
			}
		}

		int shouldBe = newDimensions.x * newDimensions.y;
		//int what = 2;

		clEnqueueWriteBuffer( queue, inSubdividePositions, CL_TRUE, 0, sizeof(glm::vec2)*shouldBe, subdividePositions, 0, 0, 0 );
	}

	

	for( int i = 0; i < newDimensions.x * newDimensions.y; i++ )
	{
		resultSet[i].a = -1;
	}
	
	
	clEnqueueWriteBuffer( queue, inInvMVMatrix, CL_TRUE, 0, sizeof(glm::mat4), &inverseMVMatrix, 0, 0, 0 );
	clEnqueueWriteBuffer( queue, inInvProjMatrix, CL_TRUE, 0, sizeof(glm::mat4), &inverseProjMatrix, 0, 0, 0 );

	error |= clSetKernelArg(vector_add_kernel, 4, sizeof(glm::vec2), &previousDimensions);
	error |= clSetKernelArg(vector_add_kernel, 5, sizeof(glm::vec4), &position);

 	/*cl_bool blocking_write,
 	size_t offset,
 	size_t cb,
 	const void *ptr,
 	cl_uint num_events_in_wait_list,
 	const cl_event *event_wait_list,s
 	cl_event *event)*/

	// Launching kernel
	const size_t local_ws = 128;	// Number of work-items per work-group
	// shrRoundUp returns the smallest multiple of local_ws bigger than size

	/*
	std::cout << "Launch " << previousDimensions.x << " " << previousDimensions.y << std::endl;
	const size_t global_ws = multipleRoundUp(local_ws, ((int)(previousDimensions.x*previousDimensions.y)));	// Total number of work-items
	error = clEnqueueNDRangeKernel(queue, vector_add_kernel, 1, NULL, &global_ws, &local_ws, 0, NULL, NULL);
	printError( "Launch Kernel", error );


	// Reading back
	std::cout << "Read Start" << std::endl;
	clEnqueueReadBuffer(queue, outResult, CL_FALSE, 0, sizeof(glm::vec4)*((int)(previousDimensions.x*previousDimensions.y)), resultSet, 0, NULL, NULL);
	std::cout << "Read End" << std::endl;
	*/

	
	const size_t amountPerGo = multipleRoundUp(local_ws, (int)(3000));
	std::cout << "Launch" << std::endl;
	for( size_t i = 0; i < newDimensions.x*newDimensions.y; i+= amountPerGo )
	{
		std::cout << "sub" << std::endl;
		//const size_t global_ws = multipleRoundUp(local_ws, ((int)(previousDimensions.x*previousDimensions.y)));	// Total number of work-items
		error = clEnqueueNDRangeKernel(queue, vector_add_kernel, 1, &i, &amountPerGo, &local_ws, 0, NULL, NULL);

		// Reading back
		clEnqueueReadBuffer(queue, outResult, CL_FALSE, sizeof(glm::vec4)*i, sizeof(glm::vec4)*((int)(newDimensions.x*newDimensions.y)), resultSet+i, 0, NULL, NULL);
	}
	std::cout << "Read Start" << std::endl;
	
}

void RayMarcherGPU::initialize()
{
	cl_int error = 0;
	/*
	//Get Platform
	cl_platform_id platform;
    cl_uint numPlatforms;
    cl_uint ok = 1;
    error = clGetPlatformIDs(ok, &platform, &numPlatforms);
	printError( "Getting Platforms", error );

	//Get Device
	cl_uint numDevices;
	error = clGetDeviceIDs( platform, CL_DEVICE_TYPE_GPU, 1, &device, &numDevices );
	printError( "Getting Devices", error );

	//Create Context
	context = clCreateContext( 0, numDevices, &device, 0, 0, &error );
	printError( "Creating Context", error );

	//Create Command-queue
	queue = clCreateCommandQueue( context, device, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_PROFILING_ENABLE, &error );
	printError( "Creating Command-queue", error );
	*/
	//Special init stuff
	init();
	//load();

	//Launching
	launch();	
}

glm::vec4 RayMarcherGPU::getPixel( int x, int y )
{
	int width = (int)previousDimensions->x;
	int cell = (y*width)+x;
	//std::cout << "x: " << x << "; y: " << y << "; dx: " << dimensions.x << "; dy: " << dimensions.y << "; cell: " << cell << std::endl;
	return resultSet[cell];
}

glm::vec4 RayMarcherGPU::getPixel( int index )
{
	return resultSet[index];
}
