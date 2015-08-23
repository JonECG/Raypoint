__kernel void vector_add_gpu (__global const float* src_a,
                     __global const float* src_b,
                     __global float* res,
		   const int num)
{
   /* get_global_id(0) returns the ID of the thread in execution.
   As many threads are launched at the same time, executing the same kernel,
   each one will receive a different ID, and consequently perform a different computation.*/
   const int idx = get_global_id(0);
   
   typedef float4 vec4;
   
   /*
		glm::vec4 mousePosition = glm::vec4( x, y,-1, 1 );
		glm::vec4 mouseEye = getInvProjection()  * mousePosition;
		mouseEye.z = -1;
		mouseEye.w = 0;
		glm::vec3 rayDirection = glm::normalize( glm::vec3( getInvModelView() * mouseEye ) );

		return Ray( from, rayDirection );
	*/

   /* Now each work-item asks itself: "is my ID inside the vector's range?"
   If the answer is YES, the work-item performs the corresponding computation*/
   if (idx < num)
      res[idx] = src_a[idx] + src_b[idx];
}