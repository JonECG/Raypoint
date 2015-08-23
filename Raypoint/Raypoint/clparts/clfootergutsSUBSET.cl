vec3 jankGetNormal( vec3 position, vec3 rayDirection, frac dist, frac limit )
{
	frac colTemp;

	vec3 tangent, bitangent, tritangent;
	bool hit = false;
	int rev = 1000;
	frac ang = 6.28301 / rev;
	bool found = true;

	//TODO: Divide and Conquer
	if( found )
	{
		found = false;
		for( int i = 0; i < rev && !found; i++ )
		{
			bool hitting = subsetFunction( position.x + dist*cos( ang * i ), position.y + dist*sin( ang * i ), position.z + dist*sin( ang * i * 3 ), &colTemp, &colTemp, &colTemp );
			if( i > 0 && hit != hitting )
			{
				tangent = normalize( (vec3)( dist*cos( ang * (i-.5) ), dist*sin( ang * (i-.5) ), dist*sin( ang * (i-.5) * 3 ) ) );
				found = true;
			}
			hit = hitting;
		}
	}

	if( found )
	{
		found = false;
		for( int i = 0; i < rev && !found; i++ )
		{
			bool hitting = subsetFunction( position.x + dist*cos( ang * i * 2 ), position.y + dist*cos( ang * i ), position.z + dist*sin( ang * i ), &colTemp, &colTemp, &colTemp );
			if( i > 0 && hit != hitting )
			{
				bitangent = normalize( (vec3)( dist*cos( ang * (i-.5) * 2 ), dist*cos( ang * (i-.5) ), dist*sin( ang * (i-.5) ) ) );
				found = true;
			}
			hit = hitting;
		}
	}
	
	if( found )
	{
		found = false;
		for( int i = 0; i < rev && !found; i++ )
		{
			bool hitting = subsetFunction( position.x + dist*sin( ang * i ), position.y + dist*cos( ang * i * 2 ), position.z + dist*cos( ang * i ), &colTemp, &colTemp, &colTemp );
			if( i > 0 && hit != hitting )
			{
				tritangent = normalize( (vec3)( dist*sin( ang * (i-.5) ), dist*cos( ang * (i-.5) * 2 ), dist*cos( ang * (i-.5) ) ) );
				found = true;
			}
			hit = hitting;
		}
	}

	vec3 normal = (vec3)( 0, 1, 0 );
	if( found )
	{
		vec3 norm12 = normalize( cross( tangent, bitangent ) );
			if( dot( norm12, rayDirection ) < 0 ) norm12 = -norm12;
		vec3 norm13 = normalize( cross( tangent, tritangent ) );
			if( dot( norm13, rayDirection ) < 0 ) norm13 = -norm13;
		vec3 norm23 = normalize( cross( bitangent, tritangent ) );
			if( dot( norm23, rayDirection ) < 0 ) norm23 = -norm23;

		normal = ( dot( norm12, norm13 ) > 0.9999 ) ? norm13 : norm23 ;
	}
	return normal;
	/*
	vec3 normal = (vec3)( 0, 1, 0 );
	if( found )
	{
		normal = normalize( cross( tangent, bitangent ) );

		if( dot( normal, rayDirection ) < 0 )
			normal = -normal;
			
		//normal = (vec3)( 1, 0, 0 );
	}
	
	return normal;
	*/
}

bool smartSubset( frac x, frac y, frac z, frac dx, frac dy, frac dz, frac stepping, frac * r, frac * g, frac * b, frac * dist )
{
	bool result = subsetFunction( x, y, z, r, g, b );
	
	if( result )
	{
		*dist = 0;
		
		int nStep = 0;
		
		for( int i = 0; i < 51; i++ )
		{
			bool retracting = (i % 2 == 0);
			int multAmount = (retracting) ? -1 : 1;
			stepping /= 2;
			nStep = 0;
			do
			{
				nStep++;
				*dist += multAmount * stepping;
				x += dx * multAmount * stepping;
				y += dy * multAmount * stepping;
				z += dz * multAmount * stepping;
			}
			while( nStep < 2 && subsetFunction( x, y, z, r, g, b ) == retracting );
		}
	}
	
	return result;
}

__kernel void ray_march_gpu (__global const float* invProjMatrix,
						__global const float* invMVMatrix,
						__global const float2* subdividePositions,
						__global const float* sceneVarData,
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
		frac currentColorRed = 1, currentColorGreen = 0, currentColorBlue = 0;
		vec4 remainingColor = (vec4)( 1, 1, 1, 1 );
		
		
		int maxMarches = 10000;
		int marches = 0;
		frac leastMarch = 0.0005;
		frac dist = 0;
		frac prevDistTo;
		frac currentDistTo = 0;//distanceFunction( currentRay.x, currentRay.y, currentRay.z, 0, 0 );
		vec3 currentRay = newPosition.xyz;
		frac totalDist = 0;
		
		frac smallestSubsetMarch = 0.0001;
		frac currentSubsetStep = smallestSubsetMarch; 
			


		while( marches < maxMarches && ( remainingColor.x > 0.01 || remainingColor.y > 0.01 || remainingColor.z > 0.01 ) )
		{			
			bool intersect = false;

			while( marches < maxMarches && !intersect )
			{
				prevDistTo = currentDistTo;
				currentDistTo = currentSubsetStep;
				
				//currentDistTo = distanceFunction( currentRay.x, currentRay.y, currentRay.z, &currentColorRed, &currentColorGreen, &currentColorBlue );
				bool hitIt = smartSubset( currentRay.x, currentRay.y, currentRay.z, rayDirection.x, rayDirection.y, rayDirection.z, currentDistTo, &currentColorRed, &currentColorGreen, &currentColorBlue, &currentDistTo );
				
				//dist += currentDistTo;
				totalDist += currentDistTo;

				currentRay += currentDistTo * rayDirection;
				intersect = hitIt;//currentDistTo < 1 && currentDistTo <= totalDist*leastMarch;
				
				//currentSubsetStep += smallestSubsetMarch;
				currentSubsetStep *= 1.005;
				
				marches++;
			}
			
			if( intersect )
			{
				//vec3 normal = normalFunction( currentRay.x, currentRay.y, currentRay.z, 0, 0 );
				vec3 normal = jankGetNormal( currentRay, rayDirection, smallestSubsetMarch, leastMarch );
				
				frac reflectivity = 0;
				
				//This objects shading
				frac amount = fabs(dot( -normal, normalize( currentRay - newPosition.xyz ) ));
				//vec4 thisColor = (vec4)( hsvToRgb( (vec3)( (1 - ( (float)(marches) / maxMarches ) ), 1, 1 ) ) , 1 );
				vec4 thisColor = (vec4)( currentColorRed, currentColorGreen, currentColorBlue, 1 );
				//thisColor = (vec4)(1,0,0,1);
					//thisColor *= (1 - ( (float)(marches) / maxMarches ) ); //AO
				thisColor *= amount; //Normal
				thisColor.w = 1;
				
				color += remainingColor * thisColor * ( 1 - reflectivity );
				remainingColor *= reflectivity;
				currentRay -= rayDirection * prevDistTo*2;
				rayDirection = rayDirection - 2.0 * normal * dot( rayDirection, normal );//reflect( rayDirection, normal );
				totalDist = 0;
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


