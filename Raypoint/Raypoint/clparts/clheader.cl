#define USE_DOUBLES

#ifdef USE_DOUBLES
	#pragma OPENCL EXTENSION cl_khr_fp64: enable
	typedef double4 vec4;
	typedef double3 vec3;
	typedef double2 vec2;
	typedef double frac;
#else
	typedef float4 vec4;
	typedef float3 vec3;
	typedef float2 vec2;
	typedef float frac;
#endif
