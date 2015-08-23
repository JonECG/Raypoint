#pragma once

#include <glm\glm.hpp>
#include <glm\ext.hpp>

#include "Ray.h"

class Camera
{
	glm::dvec3 fromOldInvProj, toOldInvProj, upOldInvProj;
	float aspectOldInvProj, fovOldInvProj;

	glm::dvec3 fromOldInvMV, toOldInvMV, upOldInvMV;

	float camNear, camFar;
public:
	float aspect, fov;
	glm::dvec3 from, to, up;
	glm::mat4 proj, view, mvp, invProj, invView, invMvp;
	inline Camera()
	{
		aspect = 1.0f;
		camNear = 0.1f;
		camFar = 200.0f;
		fov = 50;
		up = glm::dvec3( 0, 1, 0 );
	}

	inline Camera( float aspect, float camNear, float camFar)
	{
		this -> aspect = aspect;
		this -> camNear = camNear;
		this -> camFar = camFar;
		fov = 50;
		up = glm::dvec3( 0, 1, 0 );
	}

	/*inline void move( glm::vec3 move )
	{
		glm::vec3 localZ = glm::normalize( to - from );
		glm::vec3 localX = glm::cross( localZ, glm::vec3( 0, 1, 0 ) );
		glm::vec3 localY = glm::cross( localX, localZ );

		glm::mat3 localTransform = glm::mat3( localX, localY, localZ );
		glm::vec3 transformMove = localTransform * move;

		from += transformMove;
		to += transformMove;
	}*/

	inline glm::mat4 calcProjection()
	{
		proj = glm::perspective(fov, aspect, camNear, camFar);
		return proj;
	}

	inline glm::mat4 getInvProjection()
	{

		if( fromOldInvProj != from || toOldInvProj != to || upOldInvProj!= up || aspectOldInvProj != aspect || fovOldInvProj != fov )
		{
			invProj = glm::inverse( calcProjection() );
			fromOldInvProj = from;
			toOldInvProj = to;
			upOldInvProj = up;
			aspectOldInvProj = aspect;
			fovOldInvProj = fov;
		}

		return invProj;
	}

	inline glm::mat4 calcModelView()
	{
		view = glm::lookAt( glm::vec3(from), glm::vec3(to), glm::vec3(up) );
		return view;
	}

	inline glm::mat4 getInvModelView()
	{
		if( fromOldInvMV != from || toOldInvMV != to || upOldInvMV != up )
		{
			invView = glm::inverse( calcModelView() );
			fromOldInvMV = from;
			toOldInvMV = to;
			upOldInvMV = up;
		}

		return invView;
	}


	inline glm::mat4 calcModelViewProjection()
	{
		mvp = calcProjection()*calcModelView();
		return mvp;
	}

	inline glm::mat4 calcInverseModelViewProjection()
	{
		return glm::inverse( calcModelViewProjection() );
	}

	//inline Ray calculateRayFromScreen( float x, float y )
	//{
	//	//Change ray

	//	glm::vec4 mousePosition = glm::vec4( x, y,-1, 1 );
	//	glm::vec4 mouseEye = getInvProjection()  * mousePosition;
	//	mouseEye.z = -1;
	//	mouseEye.w = 0;
	//	glm::vec3 rayDirection = glm::normalize( glm::vec3( getInvModelView() * mouseEye ) );

	//	return Ray( from, rayDirection );
	//}

	inline glm::dvec3 getFrom(){ return from; }
	inline glm::dvec3 getTo(){ return to; }
	inline void setAspect( float aspect ){ this -> aspect = aspect; }
	inline void setFrom( glm::dvec3 from ){ this -> from = from; }
	inline void setTo( glm::dvec3 to ){ this -> to = to; }
};