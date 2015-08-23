//#define USE_DOUBLES

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

vec3 hsvToRgb( vec3 hsv )
{
	frac chroma = hsv.z * hsv.y;
	frac primeH = hsv.x * 6;
	frac x = chroma*(1 - fabs( fmod( primeH, 2 ) - 1 ) );
	int hWhole = (int)primeH;
	
	vec3 result = (vec3)(chroma,0,x);
	
	switch( hWhole )
	{
		case 0:
			result = result.xzy;
		break;
		case 1:
			result = result.zxy;
		break;
		case 2:
			result = result.yxz;
		break;
		case 3:
			result = result.yzx;
		break;
		case 4:
			result = result.zyx;
		break;
	}
	
	result += (vec3)(hsv.z-chroma);
	
	return result;
}

//Mandelbox
void sphereFold(vec3 * z, frac * dz) 
{
	frac minRadius2 = 0.25;
	frac fixedRadius2 = 1;
	
	frac r2 = dot(*z,*z);
	if (r2<minRadius2) { 
		/*linear inner scaling*/
		frac temp = (fixedRadius2/minRadius2);
		*z *= temp;
		*dz*= temp;
	} else if (r2<fixedRadius2) { 
		/*this is the actual sphere inversion*/
		frac temp =(fixedRadius2/r2);
		*z *= temp;
		*dz*= temp;
	}
}

void boxFold(vec3 * z, frac * dz) {
	frac foldingLimit = 1;
	
	*z = clamp(*z, -foldingLimit, foldingLimit) * 2.0 - *z;
}

frac distanceFunction( frac x, frac y, frac z, frac time, vec4 * color )
{
	frac Scale = 2.2;
	int Iterations = 100;
	
	vec3 point = (vec3)(x,y,z);
	vec3 offset = point;
	frac dr = 1.0;
	
	int bail = 1000;
	
	bool continuing = true;
	
	int n;
	
	for (n = 0; n < Iterations && continuing; n++) {
		boxFold(&point,&dr);       // Reflect
		sphereFold(&point,&dr);    // Sphere Inversion
 		
                point=Scale*point + offset;  // Scale & Translate
				
                dr = dr*fabs(Scale)+1.0;
				
				continuing = dot( point,point) < 16000; 
	}
	
	*color = (vec4)(hsvToRgb( (vec3)( ((frac)(n))/Iterations,1,1) ), 1 );
	frac r = length(point);
	return r/fabs(dr);
}

//Grid
// float distanceFunction( float x, float y, float z, float time, vec4 * color )
// {
	// vec3 p = (vec3)( x, y, z );
	// frac c = 1.0f;
	// vec3 q = fabs( fmod(p,c)-0.5*c );
	
	// return min(  max( q.x, q.z ), min( max( q.x, q.y ), max( q.z, q.y ) ) ) - 0.005;

// }

// vec3 normalFunction( float x, float y, float z, float time, vec4 * color )
// {
	// vec3 p = (vec3)( x, y, z );
	// frac c = 2.1f;
	// vec3 q = fmod(p,c)-0.5*c;
	
	// /*Iunno*/
	
	// return normalize(q);
// }

//Cube
// float distanceFunction( float x, float y, float z, float time, vec4 * color )
// {
	// vec3 p = (vec3)( x, y, z );
	// frac c = 2.1f;
	// vec3 q = fmod(p,c)-0.5*c;
	
	// return length(max(fabs(q)-((vec3)(0.5)),0.0));
// }

// vec3 normalFunction( float x, float y, float z, float time, vec4 * color )
// {
	// vec3 p = (vec3)( x, y, z );
	// frac c = 2.1f;
	// vec3 q = fmod(p,c)-0.5*c;
	
	// if ( fabs( q.x ) < fabs( q.y ) )
		// q.x = 0;
	// else
		// q.y = 0;
		
	// if ( fabs( q.x ) < fabs( q.z ) )
		// q.x = 0;
	// else
		// q.z = 0;
		
	// if ( fabs( q.y ) < fabs( q.z ) )
		// q.y = 0;
	// else
		// q.z = 0;
		
	// return normalize(q);
// }

//Sphere
// float distanceFunction( float x, float y, float z, float time, vec4 * color )
// {
	// vec3 p = (vec3)( x, y, z );
	// frac c = 2.1f;
	// vec3 q = fmod(p,c)-0.5*c;
	// return max( length( q ) - 1.0f, 0.0f );
// }


vec3 normalFunction( float x, float y, float z, float time, vec4 * color )
{
	vec3 p = (vec3)( x, y, z );
	frac c = 2.1f;
	vec3 q = fmod(p,c)-0.5*c;
	return normalize(q);
}


vec3 jankGetNormal( vec3 position, vec3 rayDirection, frac dist, frac limit )
{
	// //Normals
	vec3 tangent, bitangent, normal;
	bool hit = false;
	int rev = 20;
	frac ang = 6.28301 / rev;
	vec4 colorMeWhiskersLarry;
	bool found = false;

	for( int i = 0; i < rev && !found; i++ )
	{
		bool hitting = distanceFunction( position.x + dist*cos( ang * i ), position.y + dist*sin( ang * i ), position.z + dist*sin( ang * i * 3 ), 0, &colorMeWhiskersLarry ) <= limit;
		if( i > 0 && hit != hitting )
		{
			tangent = normalize( (vec3)( dist*cos( ang * (i-.5) ), dist*sin( ang * (i-.5) ), dist*sin( ang * (i-.5) * 3 ) ) );
			found = true;
		}
		hit = hitting;
	}

	found = false;
	for( int i = 0; i < rev && !found; i++ )
	{
		bool hitting = distanceFunction( position.x + dist*cos( ang * i * 2 ), position.y + dist*cos( ang * i ), position.z + dist*sin( ang * i ), 0, &colorMeWhiskersLarry ) <= limit;
		if( i > 0 && hit != hitting )
		{
			bitangent = normalize( (vec3)( dist*cos( ang * (i-.5) * 2 ), dist*cos( ang * (i-.5) ), dist*sin( ang * (i-.5) ) ) );
			found = true;
		}
		hit = hitting;
	}

	if( found )
	{
		normal = normalize( cross( tangent, bitangent ) );

		if( dot( normal, rayDirection ) < 0 )
			normal = -normal;
	}
	else
	{
		normal = (vec3)(0,0,0);
	}
	
	return normal;
}

__kernel void ray_march_gpu (__global const float* invProjMatrix,
						__global const float* invMVMatrix,
						__global const float2* subdividePositions,
						__global float4* result,
						const float2 dimensions,
						const float4 position,
						const int offset)
{
	/* get_global_id(0) returns the ID of the thread in execution.
	As many threads are launched at the same time, executing the same kernel,
	each one will receive a different ID, and consequently perform a different computation.*/
	const int idx = get_global_id(0)+offset;
	
	vec4 newPosition = (vec4)(position.x,position.y,position.z,position.w);
	
	if( idx < (dimensions.x * dimensions.y) )
	{
		result[idx] = (float4)( 0, 0, 0.1, 1 );
		//Calculate ray
		
		frac xPixel = subdividePositions[idx].x;
		frac yPixel = subdividePositions[idx].y;
		
		frac rayFullDirection[4] = { (2*xPixel)-1, (-2*yPixel)+1, -1, 1 };
		frac surrogate[4];
		
		frac total;
		
		for( int i = 0; i < 2; i++ )
		{
			total = 0;
			for( int j = 0; j < 4; j++ )
			{
				total += invProjMatrix[j*4+i] * rayFullDirection[j];
			}
			surrogate[i] = total;
		}
		
		surrogate[2] = -1;
		surrogate[3] = 0;
		
		for( int i = 0; i < 3; i++ )
		{
			total = 0;
			for( int j = 0; j < 4; j++ )
			{
				total = total + invMVMatrix[j*4+i] * surrogate[j];
			}
			rayFullDirection[i] = total;
		}
		
		vec3 rayDirection = normalize( (vec3)(rayFullDirection[0],rayFullDirection[1],rayFullDirection[2]) );

		//March
		
		vec4 color =  (vec4)( 0, 0, 0, 1 );
		vec4 currentColor = (vec4)(1,0,0,1);
		vec4 remainingColor = (vec4)( 1, 1, 1, 1 );
		
		
		int maxMarches = 100;
		int marches = 0;
		frac leastMarch = 0.00001;//0000000001;
		frac dist = 0;
		frac prevDistTo;
		frac currentDistTo = 0;//distanceFunction( currentRay.x, currentRay.y, currentRay.z, 0, 0 );
		vec3 currentRay = newPosition.xyz;
			


		while( marches < maxMarches && ( remainingColor.x > 0.01 || remainingColor.y > 0.01 || remainingColor.z > 0.01 ) )
		{			
			bool intersect = false;

			while( marches < maxMarches && !intersect )
			{
				prevDistTo = currentDistTo;
				currentDistTo = distanceFunction( currentRay.x, currentRay.y, currentRay.z, 0, &currentColor );
				//dist += currentDistTo;
				
				currentRay += currentDistTo * rayDirection;
				intersect = currentDistTo <= leastMarch;
				
				marches++;
			}
			
			if( intersect )
			{
				vec3 normal = normalFunction( currentRay.x, currentRay.y, currentRay.z, 0, 0 );
				//vec3 normal = jankGetNormal( currentRay, rayDirection, currentDistTo*10, leastMarch );
				
				frac reflectivity = 0;
				
				//This objects shading
				frac amount = fabs(dot( -normal, normalize( currentRay - newPosition.xyz ) ));
				//vec4 thisColor = (vec4)( hsvToRgb( (vec3)( (1 - ( (float)(marches) / maxMarches ) ), 1, 1 ) ) , 1 );
				vec4 thisColor = currentColor;
				thisColor *= (1 - ( (float)(marches) / maxMarches ) ); //AO
				//thisColor *= amount; //Normal
				thisColor.w = 1;
				
				color += remainingColor * thisColor * ( 1 - reflectivity );
				remainingColor *= reflectivity;
				currentRay -= rayDirection * prevDistTo*2;
				rayDirection = rayDirection - 2.0 * normal * dot( rayDirection, normal );//reflect( rayDirection, normal );
				
			}
		}
		
		// if( intersect )
		// {
			// for( int i = 0; i < 5; i++ )
			// {
				// bool retracting = (i % 2 == 0);
				// int multamount = (retracting) ? -1 : 1;
				// incrementAmount /= 2;
				// int maxCount = 0;
				// do
				// {
					// maxCount++;
					// dist += multamount * incrementAmount;
					// currentRay = rayDirection * dist + position.xyz;
				// }
				// while( marchFunction( currentRay.x, currentRay.y, currentRay.z, 0, &color ) == retracting && maxCount < 5 );
			// }
		// }
		
		
		result[idx] = (float4)( color.x, color.y, color.z, 1 );
		
		//Point at <0,0,0> test
		//result[idx] = (vec4)( pow( dot( rayDirection , normalize(-position.xyz) ), 1000.0f ), 0, 0, 1 );
		//Where is ray
		//result[idx] = (vec4)( 1+rayDirection.x , 1+rayDirection.y, 1+rayDirection.z, 1 );
		//Screen pick test
		//result[idx] = (vec4)( xPixel, yPixel, 1, 1 );
		//Actual
		//result[idx] = (vec4)( 1-dist/maxDist, 1-dist/maxDist, 1-dist/maxDist, 1 );
		
	}
}


