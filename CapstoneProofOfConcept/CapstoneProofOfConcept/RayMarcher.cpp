#include "RayMarcher.h"


RayMarcher::RayMarcher(std::function<bool(float,float,float,float)> targetFunction, Camera * camera)
{
	this->targetFunction = targetFunction;
	this->camera = camera;
}

bool RayMarcher::marchRay( float x, float y, float * distance, glm::vec3 * normal )
{
	Ray ray = camera->calculateRayFromScreen( x, y );

	float dist = 0;
	float incrementAmount = 0.03f;

	glm::vec3 currentPos;

	bool intersect = false;

	while( dist < 5 && !intersect )
	{
		dist += incrementAmount;
		currentPos = ray.position + ray.direction * dist;
		intersect = targetFunction( currentPos.x, currentPos.y, currentPos.z, 0 );
	}

	if( intersect )
	{
		for( int i = 0; i < 13; i++ )
		{
			bool retracting = (i % 2 == 0);
			int multAmount = (retracting) ? -1 : 1;
			incrementAmount /= 2;
			do
			{
				dist += multAmount * incrementAmount;
				currentPos = ray.position + ray.direction * dist;
			}
			while( targetFunction( currentPos.x, currentPos.y, currentPos.z, 0 ) == retracting );
		}

		*distance = dist;
		
		incrementAmount = 0.001;

		glm::vec3 tangent, bitangent;
		bool hit = false;
		int rev = 500;
		float ang = 6.28301 / rev;
		bool found = false;

		for( int i = 0; i < rev && !found; i++ )
		{
			bool hitting = targetFunction( currentPos.x + incrementAmount*std::cos( ang * i ), currentPos.y + incrementAmount*std::sin( ang * i ), currentPos.z + incrementAmount*std::sin( ang * i * 3 ), 0 );
			if( i > 0 && hit != hitting )
			{
				tangent = glm::normalize( glm::vec3( incrementAmount*std::cos( ang * (i-.5) ), incrementAmount*std::sin( ang * (i-.5) ), incrementAmount*std::sin( ang * (i-.5) * 3 ) ) );
				found = true;
			}
			hit = hitting;
		}

		found = false;
		for( int i = 0; i < rev && !found; i++ )
		{
			bool hitting = targetFunction( currentPos.x + incrementAmount*std::cos( ang * i * 2 ), currentPos.y + incrementAmount*std::cos( ang * i ), currentPos.z + incrementAmount*std::sin( ang * i ), 0 );
			if( i > 0 && hit != hitting )
			{
				bitangent = glm::normalize( glm::vec3( incrementAmount*std::cos( ang * (i-.5) * 2 ), incrementAmount*std::cos( ang * (i-.5) ), incrementAmount*std::sin( ang * (i-.5) ) ) );
				found = true;
			}
			hit = hitting;
		}

		if( found )
		{
			*normal = glm::normalize( glm::cross( tangent, bitangent ) );

			if( glm::dot( *normal, ray.direction ) < 0 )
				*normal = -*normal;
			//std::cout << normal->x << " " << normal->y << " " << normal->z << std::endl;
		}
	}

	return intersect;
}
