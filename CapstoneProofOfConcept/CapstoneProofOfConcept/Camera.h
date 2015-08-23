#pragma once

#include <glm\glm.hpp>
#include <glm\ext.hpp>

#include "Ray.h"

class Camera
{
	
	float camNear, camFar;
public:
	float aspect;
	glm::vec3 from, to;
	glm::mat4 proj, view, mvp, invProj, invView, invMvp;
	inline Camera()
	{
		aspect = 1.0f;
		camNear = 0.1f;
		camFar = 200.0f;
	}

	inline Camera( float aspect, float camNear, float camFar)
	{
		this -> aspect = aspect;
		this -> camNear = camNear;
		this -> camFar = camFar;
	}

	inline void move( glm::vec3 move )
	{
		glm::vec3 localZ = glm::normalize( to - from );
		glm::vec3 localX = glm::cross( localZ, glm::vec3( 0, 1, 0 ) );
		glm::vec3 localY = glm::cross( localX, localZ );

		glm::mat3 localTransform = glm::mat3( localX, localY, localZ );
		glm::vec3 transformMove = localTransform * move;

		from += transformMove;
		to += transformMove;
	}

	inline glm::mat4 calcProjection()
	{
		proj = glm::perspective(50.0f, aspect, camNear, camFar);
		return proj;
	}

	inline glm::mat4 getInvProjection()
	{
		static glm::vec3 fromOld, toOld;
		static float aspectOld;

		if( fromOld != from || toOld != to || aspectOld != aspect )
		{
			invProj = glm::inverse( calcProjection() );
			fromOld = from;
			toOld = to;
			aspectOld = aspect;
		}

		return invProj;
	}

	inline glm::mat4 calcModelView()
	{
		view = glm::lookAt( from, to, glm::vec3( 0.0, 1.0, 0.0 ) );
		return view;
	}

	inline glm::mat4 getInvModelView()
	{
		static glm::vec3 fromOld, toOld;
		static float aspectOld;

		if( fromOld != from || toOld != to || aspectOld != aspect )
		{
			invView = glm::inverse( calcModelView() );
			fromOld = from;
			toOld = to;
			aspectOld = aspect;
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

	inline Ray calculateRayFromScreen( float x, float y )
	{
		//Change ray

		glm::vec4 mousePosition = glm::vec4( x, y,-1, 1 );
		glm::vec4 mouseEye = getInvProjection()  * mousePosition;
		mouseEye.z = -1;
		mouseEye.w = 0;
		glm::vec3 rayDirection = glm::normalize( glm::vec3( getInvModelView() * mouseEye ) );

		return Ray( from, rayDirection );
	}

	inline glm::vec3 getFrom(){ return from; }
	inline glm::vec3 getTo(){ return to; }
	inline void setAspect( float aspect ){ this -> aspect = aspect; }
	inline void setFrom( glm::vec3 from ){ this -> from = from; }
	inline void setTo( glm::vec3 to ){ this -> to = to; }
};