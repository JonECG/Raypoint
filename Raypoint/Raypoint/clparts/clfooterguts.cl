vec3 jankGetNormal( vec3 position, vec3 rayDirection, frac dist, frac limit, __global const float* sceneVarData )
{
	frac colTemp;
	return normalize( (vec3)(
		distanceFunction(position.x+dist,position.y,position.z, &colTemp, &colTemp, &colTemp, sceneVarData ) - distanceFunction(position.x-dist,position.y,position.z, &colTemp, &colTemp, &colTemp, sceneVarData ),
		distanceFunction(position.x,position.y+dist,position.z, &colTemp, &colTemp, &colTemp, sceneVarData ) - distanceFunction(position.x,position.y-dist,position.z, &colTemp, &colTemp, &colTemp, sceneVarData ),
		distanceFunction(position.x,position.y,position.z+dist, &colTemp, &colTemp, &colTemp, sceneVarData ) - distanceFunction(position.x,position.y,position.z-dist, &colTemp, &colTemp, &colTemp, sceneVarData ) ) );
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
		
		
		int maxMarches = 300;
		int marches = 0;
		frac leastMarch = 0.0005;
		frac dist = 0;
		frac prevDistTo;
		frac currentDistTo = 0;//distanceFunction( currentRay.x, currentRay.y, currentRay.z, 0, 0 );
		vec3 currentRay = newPosition.xyz;
		frac totalDist = 0;
			


		while( marches < maxMarches && ( remainingColor.x > 0.01 || remainingColor.y > 0.01 || remainingColor.z > 0.01 ) )
		{			
			bool intersect = false;

			while( marches < maxMarches && !intersect )
			{
				prevDistTo = currentDistTo;
				currentDistTo = distanceFunction( currentRay.x, currentRay.y, currentRay.z, &currentColorRed, &currentColorGreen, &currentColorBlue, sceneVarData );
				
				//dist += currentDistTo;
				totalDist += currentDistTo;

				currentRay += currentDistTo * rayDirection;
				intersect = currentDistTo < 1 && currentDistTo <= totalDist*leastMarch;
				
				
				marches++;
			}
			
			if( intersect )
			{
				//vec3 normal = normalFunction( currentRay.x, currentRay.y, currentRay.z, 0, 0 );
				vec3 normal = jankGetNormal( currentRay, rayDirection, leastMarch/2, leastMarch, sceneVarData );
				
				frac reflectivity = 0;
				
				//This objects shading
				frac amount = fabs(dot( -normal, normalize( currentRay - newPosition.xyz ) ));
				//vec4 thisColor = (vec4)( hsvToRgb( (vec3)( (1 - ( (float)(marches) / maxMarches ) ), 1, 1 ) ) , 1 );
				vec4 thisColor = (vec4)( currentColorRed, currentColorGreen, currentColorBlue, 1 );
				//thisColor = (vec4)(1,0,0,1);
				thisColor *= (1 - ( (float)(marches) / maxMarches ) ); //AO
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


