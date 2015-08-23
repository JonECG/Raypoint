typedef float4 vec4;
typedef float3 vec3;
typedef float2 vec2;
typedef float frac;

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

bool marchFunction( float x, float y, float z, float time, vec4 * color )
{
	//return (x*x+y*y+z*z)<2;
	//x = x + sin( y * 5 )*z;
	int iterations = 3;//(int)( length( (vec3)(x,y,z) ) * 3 + 2);

	bool ultResult = ( x > 0 && x < 1 && y > 0 && y < 1 && z > 0 && z < 1 );
	bool result = true;
	
	int i;
	for( i = 1; i < iterations && result && ultResult; i++ )
	{
		float product = pow( 3.0f, (float)i );
		int count = 0;

		if (((int)(product * x) % 3)==1)
			count++;
		if (((int)(product * y) % 3)==1)
			count++;
		if (((int)(product * z) % 3)==1)
			count++;

		result = result && count < 2;
	}
	
	if( color != 0 && ultResult && !result )
	{
		frac amount = ((frac)i-2)/(iterations);
		// (*color).x = amount;
		// (*color).y = amount;
		// (*color).z = amount;
		(*color) = (vec4)( hsvToRgb( (vec3)(amount,1,1) ), 1 );
	}

	return ultResult && !result;
}

vec3 ballFold( float radius, vec3 vec )
{
	/*
	if m<r         m = m/r^2
	  else if m<1 m = 1/m
	*/
	frac l2 = dot( vec, vec );
	if( l2 < radius * radius )
		return vec / (radius * radius);
	if( l2 < 1 )
		return vec / length( vec );
	return vec;
}

vec3 boxFold( vec3 vec )
{
	/*
	if v[a]>1          v[a] =  2-v[a]
	  else if v[a]<-1 v[a] =-2-v[a]
	*/
	// for( int i = 0; i < 3; i++ )
	// {
		// if ( vec[i] > 1 )
			// vec[i] = 2 - vec[i];
		// else
		// if ( vec[i] < -1 )
			// vec[i] = -2 - vec[i];
	// }
	
	vec3 result;
	
	if ( vec.x > 1 )
		result.x = 2 - vec.x;
	else
	if ( vec.x < -1 )
		result.x = -2 - vec.x;
		
	if ( vec.y > 1 )
		result.y = 2 - vec.y;
	else
	if ( vec.y < -1 )
		result.y = -2 - vec.y;
		
	if ( vec.z > 1 )
		result.z = 2 - vec.z;
	else
	if ( vec.z < -1 )
		result.z = -2 - vec.z;
			
	return result;
}

bool marchaFunction( float x, float y, float z, float time, vec4 * color )
{ 
	/*
	In fact it replaces the Mandelbrot equation z = z2 + c with:  
	  v = s*ballFold(r, f*boxFold(v)) + c
	where boxFold(v) means for each axis a:
	  if v[a]>1          v[a] =  2-v[a]
	  else if v[a]<-1 v[a] =-2-v[a]
	and ballFold(r, v) means for v's magnitude m:
	  if m<r         m = m/r^2
	  else if m<1 m = 1/m
	*/
	const int maxIterations = 25;
	int iteration = 0;
	
	
	bool result = z < 1;
	z = 0;

	vec3 com = (vec3)( 0, 0, 0 );
	float s = 2, r = 0.5, f = 1, bail = 256;

	for( iteration = 0; iteration < maxIterations && result; iteration++ )
	{
		com = s*ballFold(r, f*boxFold(com)) + (vec3)(x,y,z);

		frac l2 = dot( com, com );
		result = l2 < bail;

		// if( l2 < 0.001 )
			// return true;
	}
	
	if( color != 0 )
	{
		frac amount = ((frac)iteration)/maxIterations;
		// (*color).x = amount;
		// (*color).y = amount;
		// (*color).z = amount;
		(*color) = (vec4)( hsvToRgb( (vec3)(amount,1,1) ), 1 );
	}

	return iteration > 2 && iteration < maxIterations;//result;

}


__kernel void ray_march_gpu (__global const float* invProjMatrix,
						__global const float* invMVMatrix,
						__global const float2* subdividePositions,
						__global float4* result,
						const float2 dimensions,
						const float4 position)
{
	/* get_global_id(0) returns the ID of the thread in execution.
	As many threads are launched at the same time, executing the same kernel,
	each one will receive a different ID, and consequently perform a different computation.*/
	const int idx = get_global_id(0);
	
	if( idx < (dimensions.x * dimensions.y) )
	{
		result[idx] = (vec4)( 0, 0, 0.1, 1 );
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
		
		vec4 color =  (vec4)( 1, 0, 0, 1 );
		
		frac dist = 0;
		frac incrementAmount = 0.01;
		vec3 rayPart = incrementAmount * rayDirection;
		frac maxDist = 4.0;
		
		vec3 currentRay = position.xyz;
		
		bool intersect = false;

		while( dist < maxDist && !intersect )
		{
			dist += incrementAmount;
			currentRay += rayPart;
			intersect = marchFunction( currentRay.x, currentRay.y, currentRay.z, 0, 0 );
		}
		
		if( intersect )
		{
			for( int i = 0; i < 5; i++ )
			{
				bool retracting = (i % 2 == 0);
				int multamount = (retracting) ? -1 : 1;
				incrementAmount /= 2;
				int maxCount = 0;
				do
				{
					maxCount++;
					dist += multamount * incrementAmount;
					currentRay = rayDirection * dist + position.xyz;
				}
				while( marchFunction( currentRay.x, currentRay.y, currentRay.z, 0, &color ) == retracting && maxCount < 5 );
			}
		}
		
		
		//Normals
		vec3 tangent, bitangent, normal;
		bool hit = false;
		int rev = 200;
		frac ang = 6.28301 / rev;
		bool found = false;
		
		incrementAmount *= 8;

		for( int i = 0; i < rev && !found; i++ )
		{
			bool hitting = marchFunction( currentRay.x + incrementAmount*cos( ang * i ), currentRay.y + incrementAmount*sin( ang * i ), currentRay.z + incrementAmount*sin( ang * i * 3 ), 0, 0 );
			if( i > 0 && hit != hitting )
			{
				tangent = normalize( (vec3)( incrementAmount*cos( ang * (i-.5) ), incrementAmount*sin( ang * (i-.5) ), incrementAmount*sin( ang * (i-.5) * 3 ) ) );
				found = true;
			}
			hit = hitting;
		}

		found = false;
		for( int i = 0; i < rev && !found; i++ )
		{
			bool hitting = marchFunction( currentRay.x + incrementAmount*cos( ang * i * 2 ), currentRay.y + incrementAmount*cos( ang * i ), currentRay.z + incrementAmount*sin( ang * i ), 0, 0 );
			if( i > 0 && hit != hitting )
			{
				bitangent = normalize( (vec3)( incrementAmount*cos( ang * (i-.5) * 2 ), incrementAmount*cos( ang * (i-.5) ), incrementAmount*sin( ang * (i-.5) ) ) );
				found = true;
			}
			hit = hitting;
		}

		if( found )
		{
			normal = normalize( cross( tangent, bitangent ) );

			if( dot( normal, rayDirection ) < 0 )
				normal = -normal;
				
			frac amount = dot( normal, rayDirection );
			result[idx] = (vec4)( color.x * amount, color.y * amount, color.z * amount, 1 );
		}
		else
		if( dist < maxDist )
			result[idx] = (vec4)( color.x, color.y, color.z, 1 );
		
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


