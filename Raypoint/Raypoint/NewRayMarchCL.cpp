#include "NewRayMarchCL.h"

#include <iostream>
#include <fstream>
#include <thread>
#include <glm\glm.hpp>

NewRayMarchCL::NewRayMarchCL()
{
	this->camera = new Camera;
	rewriteSubs = false;
	runningThread = false;
	quitFlag = false;
	calculatedAmount = 0;
	programAlreadyLoaded = false;
}

void printCLError( const char * location, cl_int error )
{
	if (error != CL_SUCCESS) 
	{
	   std::cout << "ERROR \"" << location << "\": " << error << std::endl;
	}
}

std::size_t getFilesize(const char* filename)
{
	std::ifstream in(filename, std::ifstream::in | std::ifstream::binary);
    in.seekg(0, std::ifstream::end);
    std::size_t result = size_t( in.tellg() ); 

	in.close();
    return result; 
}

char* getFileContents( const char* filename, std::size_t * fileSize )
{
	*fileSize = getFilesize( filename ) + 1;
	char * result = new char[ *fileSize ];
	result[ *fileSize - 1 ] = '\0';

	std::ifstream in(filename, std::ios::binary);
	in.read(result, *fileSize - 1);
	in.close();

	return result;
}

int roundUpByMultiple( int multiple, int target )
{
	return multiple * (target/multiple + 1);
}


void NewRayMarchCL::init()
{
	//Allocate Memory
	inverseMVMatrix = camera->getInvModelView();
	inverseProjMatrix = camera->getInvProjection();
	std::cout << sizeof( glm::vec4 ) * MWIDTH*MHEIGHT << std::endl;
	resultSet = new glm::vec4[MWIDTH*MHEIGHT];
	subdividePositions = new glm::vec2[MWIDTH*MHEIGHT];
	subdivideSizes = new glm::vec2[MWIDTH*MHEIGHT];
	previousDimensions = new glm::vec2( 0, 0 );
	position = glm::vec4( camera->from, 0 );
}

void NewRayMarchCL::unload()
{
	stopWait();
	clFinish( queue );
	clReleaseProgram(program);
	clReleaseKernel(rayKernel);
	clReleaseMemObject(inInvMVMatrix);
	clReleaseMemObject(inInvProjMatrix);
	clReleaseMemObject(inSubdividePositions);
	clReleaseMemObject(outResult);
	programAlreadyLoaded = false;
}

void NewRayMarchCL::load( const char * filename )
{
	if( programAlreadyLoaded )
	{
		unload();
	}

	rewriteSubs = true;
	cl_int overError = 0, error = 0;
	//int size = sized;

	//Create Buffers
	inInvProjMatrix = clCreateBuffer( context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof( glm::mat4 ), &inverseProjMatrix, &error );
	inInvMVMatrix = clCreateBuffer( context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof( glm::mat4 ), &inverseMVMatrix, &error );
	inSubdividePositions = clCreateBuffer( context, CL_MEM_READ_ONLY, sizeof( glm::vec2 ) * MWIDTH*MHEIGHT, 0, &error );
	inData = clCreateBuffer( context, CL_MEM_READ_ONLY, sizeof( float ) * MDATUMS, 0, &error );
	outResult = clCreateBuffer( context, CL_MEM_WRITE_ONLY, sizeof( glm::vec4 ) * MWIDTH*MHEIGHT, 0, &error );

	std::cout << "Compiling started" << std::endl;

	// Creates the program
	size_t src_size = 0;

	const char * source = getFileContents( filename, &src_size );
	
	program = clCreateProgramWithSource(context, 1, &source, &src_size, &error);
	//std::cout << source << std::endl;
	
	delete source;

	printCLError( "Creating Program", error );
	
	// Builds the program
	error = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
	overError |= error;
	printCLError( "Building Program", error );


	// Extracting the kernel
	rayKernel = clCreateKernel(program, "ray_march_gpu", &error);
	overError |= error;
	printCLError( "Extract Kernel", error );

	//Set Up Arguments
	// Note that we inform the size of the cl_mem object, not the size of the memory pointed by it
	error = clSetKernelArg(rayKernel, 0, sizeof(cl_mem), &inInvProjMatrix);
	error |= clSetKernelArg(rayKernel, 1, sizeof(cl_mem), &inInvMVMatrix);
	error |= clSetKernelArg(rayKernel, 2, sizeof(cl_mem), &inSubdividePositions);
	error |= clSetKernelArg(rayKernel, 3, sizeof(cl_mem), &inData);
	error |= clSetKernelArg(rayKernel, 4, sizeof(cl_mem), &outResult);
	overError |= error;
	printCLError( "Set up arguments", error );

	if( overError )
	{
		// Shows the log
		char* build_log;
		size_t log_size;
		// First call to know the proper size
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
		build_log = new char[log_size+1];
		// Second call to get the log
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_size, build_log, NULL);
		build_log[log_size] = '\0';

		throw build_log;

		//std::cout << build_log << std::endl;
		delete[] build_log;
	}

	programAlreadyLoaded = true;
	std::cout << "Compiling ended" << std::endl;
}

void NewRayMarchCL::stopWait()
{
	if( runningThread )
	{
		quitFlag = true;
		while( runningThread )
		{
			std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
		}
		quitFlag = false;
	}
}

void doStuff( NewRayMarchCL * marchear, cl_kernel kernel, cl_command_queue queue, cl_mem kernelData, void* cpuData )
{
	// Launching kernel
	const size_t local_ws = 128;
	
	size_t amountPerGo = size_t(marchear->previousDimensions->x*marchear->previousDimensions->y);
	while( amountPerGo > 200 )
		amountPerGo = amountPerGo >> 2;
	amountPerGo = roundUpByMultiple(local_ws, amountPerGo);

	size_t constSize = amountPerGo;

	//std::cout << "Launch" << std::endl;

	//for( int j = 0; j < 10; j++ )
	//{
	for( size_t i = 0; i < marchear->previousDimensions->x*marchear->previousDimensions->y && (i<1||!marchear->quitFlag); i+= constSize )
	{
		constSize += local_ws;
		clSetKernelArg(kernel, 7, sizeof(int), &i);
		//std::cout << "sub" << std::endl;

		cl_int error = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &constSize, &local_ws, 0, NULL, NULL);

		//std::cout<< "Queued" << i << "," << marchear->previousDimensions->x*marchear->previousDimensions->y << std::endl;
		//std::cout<< marchear->previousDimensions->y << std::endl;
		// Reading back
		clEnqueueReadBuffer(queue, kernelData, CL_TRUE, sizeof(glm::vec4)*i, sizeof(glm::vec4)*(constSize), reinterpret_cast<char*>(cpuData)+sizeof(glm::vec4)*i, 0, NULL, NULL);
		marchear->calculatedAmount += constSize;
	}
	//}
	
	//std::cout << "End launch Start" << std::endl;

	marchear->runningThread = false;
}

struct Rect{
	short x, y, w, h;
	bool update;
	Rect(){}
	Rect( short x, short y, short w, short h, bool update ){
		this->x = x;
		this->y = y;
		this->w = w;
		this->h = h;
		this->update = update;
	}
};

bool NewRayMarchCL::launch( glm::vec2 dimensions )
{
	if( runningThread )
	{
		quitFlag = true;
		return false;
	}
	else
	{
		calculatedAmount = 0;
		quitFlag = false;
	}

	cl_int error = 0;
	int size = sized;

	clFinish( queue );

	inverseMVMatrix = camera->getInvModelView();
	inverseProjMatrix = camera->getInvProjection();
	position = glm::vec4( camera->from, 0 );
	glm::vec2 oldDim = *previousDimensions;

	//std::cout << "Comp " << previousDimensions->x << " " << newDimensions.x << std::endl;

	if( oldDim.x != dimensions.x || oldDim.y != dimensions.y )
	{
		rewriteSubs = true;
		std::cout << "Dis " << oldDim.x << " " << dimensions.x << std::endl;
		*previousDimensions = dimensions;
		
		int queueIndex = 0;
		int queueCount = 0;
		Rect * queue = new Rect[ int(dimensions.x * dimensions.y * 2 ) ];

		queue[ queueCount++ ] = Rect( 0, 0, (short)dimensions.x, (short)dimensions.y, true );

		int currentIndex = 0;

		while( queueIndex < queueCount )
		{
			Rect next = queue[ queueIndex++ ];

			if( next.update )
			{
				subdividePositions[currentIndex] = glm::vec2( next.x / dimensions.x, next.y / dimensions.y );
				subdivideSizes[currentIndex] = glm::vec2( next.w / dimensions.x, next.h / dimensions.y );
				currentIndex++;
			}

			bool hasRight = next.w > 1;
			bool hasBottom = next.h > 1;

			if( hasRight || hasBottom ) //Top left
				queue[ queueCount++ ] = Rect( next.x, next.y, (hasRight) ? (next.w+1)/2 : next.w, (hasBottom) ? (next.h+1)/2 : next.h, false );
			if( hasBottom ) //Bottom left
				queue[ queueCount++ ] = Rect( next.x, next.y + (next.h+1)/2, (hasRight) ? (next.w+1)/2 : next.w, next.h/2, true );
			if( hasRight ) //Top right
				queue[ queueCount++ ] = Rect( next.x + (next.w+1)/2, next.y, next.w/2, (hasBottom) ? (next.h+1)/2 : next.h, true );
			if( hasRight && hasBottom ) //Bottom right
				queue[ queueCount++ ] = Rect( next.x + ( next.w + 1 )/2, next.y + (next.h+1)/2, next.w/2, next.h/2, true );
		}

		delete queue;
		
	}
	if( rewriteSubs )
	{
		int shouldBe = int(dimensions.x * dimensions.y);
		//int what = 2;
		clEnqueueWriteBuffer( queue, inSubdividePositions, CL_TRUE, 0, sizeof(glm::vec2)*shouldBe, subdividePositions, 0, 0, 0 );
		rewriteSubs = false;
	}

	

	for( int i = 0; i < dimensions.x * dimensions.y; i++ )
	{
		resultSet[i].a = -1;
	}
	
	
	clEnqueueWriteBuffer( queue, inInvMVMatrix, CL_TRUE, 0, sizeof(glm::mat4), &inverseMVMatrix, 0, 0, 0 );
	clEnqueueWriteBuffer( queue, inInvProjMatrix, CL_TRUE, 0, sizeof(glm::mat4), &inverseProjMatrix, 0, 0, 0 );

	error |= clSetKernelArg(rayKernel, 5, sizeof(glm::vec2), &dimensions);
	error |= clSetKernelArg(rayKernel, 6, sizeof(glm::vec4), &position);

 	/*cl_bool blocking_write,
 	size_t offset,
 	size_t cb,
 	const void *ptr,
 	cl_uint num_events_in_wait_list,
 	const cl_event *event_wait_list,s
 	cl_event *event)*/

	

	runningThread = true;
	std::thread t1( doStuff, this, rayKernel, queue, outResult, resultSet );
	t1.detach();	
	
	return true;
}

void NewRayMarchCL::initialize()
{
	cl_int error = 0;
	
	//Get Platform
	cl_platform_id * platforms = new cl_platform_id[2];
    cl_uint numPlatforms;
    cl_uint numPlatformsToCheck = 2;
	error = clGetPlatformIDs(numPlatformsToCheck, platforms, &numPlatforms);
	printCLError( "Getting Platforms", error );

	//Get Device
	cl_uint numDevices;
	error = clGetDeviceIDs( platforms[0], CL_DEVICE_TYPE_GPU, 1, &device, &numDevices );
	printCLError( "Getting Devices", error );

	//Create Context
	context = clCreateContext( 0, numDevices, &device, 0, 0, &error );
	printCLError( "Creating Context", error );

	//Create Command-queue
	queue = clCreateCommandQueue( context, device, 0 /*CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_PROFILING_ENABLE*/, &error );
	printCLError( "Creating Command-queue", error );
	
	//Special init stuff
	init();
}

glm::vec4 NewRayMarchCL::getPixel( int x, int y )
{
	int width = (int)previousDimensions->x;
	int cell = (y*width)+x;
	//std::cout << "x: " << x << "; y: " << y << "; dx: " << dimensions.x << "; dy: " << dimensions.y << "; cell: " << cell << std::endl;
	return resultSet[cell];
}

glm::vec4 NewRayMarchCL::getPixel( int index )
{
	return resultSet[index];
}


void NewRayMarchCL::setData( int index, float data )
{
	setData( index, &data, 1 );
}
void NewRayMarchCL::setData( int index, float * data, int count )
{
	clEnqueueWriteBuffer( queue, inData, CL_TRUE, index * sizeof( float ), count * sizeof( float ), data, 0, 0, 0 );
}

bool NewRayMarchCL::isComplete()
{
	return !runningThread;
}