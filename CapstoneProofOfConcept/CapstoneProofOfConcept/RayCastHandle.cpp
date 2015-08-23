#include "RayCastHandle.h"

#include <functional>

#include <iostream>
#include "Camera.h"

#include "glm\glm.hpp"
#include "RayMarcher.h"
#include "RayMarcherGPU.h"

#include "Qt\qpainter.h"
#include "Qt\qimage.h"
#include <thread>

#include <sys/types.h>
#include <sys/stat.h> 


#include <Windows.h>
#include <Qt\qapplication.h>
#include <mutex>



Camera cam( 1, 0.1, 10 );
RayMarcher * marcher;

QImage * image;
RayMarcherGPU * gpu;
std::mutex imageHold;

bool cubeTest( float x, float y, float z, float time )
{
	return( x > -1 && x < 1 && y > -1 && y < 1 && z > -1 && z < 1 );
}

bool sphereTest( float x, float y, float z, float time )
{
	return( glm::length2( glm::vec3(x,y,z) ) < 1 );// && (((int)(x * 50)%4) == 0) );
}

bool mandelboxTestA( float x, float y, float z, float time )
{
	/*
	function iterate(z):
    for each component in z:
        if component > 1:
            component := 2 - component
        else if component < -1:
            component := -2 - component

    if magnitude of z < 0.5:
        z := z * 4
    else if magnitude of z < 1:
        z := z / (magnitude of z)^2
   
    z := scale * z + c
	*/
	int iterations = 3;

	bool result = true;

	glm::vec3 com(0,0,0);

	for( int i = 0; i < iterations && result; i++ )
	{
		for( int j = 0; j < 3; j++ )
		{
			if( com[j] > 1 )
				com[j] = 2 - com[j];
			else
			if( com[j] < -1 )
				com[j] = -2 - com[j];
		}
		if ( glm::length2( com ) < 0.25 )
			com *= 4.0f;
		else
		if ( glm::length2( com ) < 1 )
			com /= glm::length2( com );

		com = 2.0f * com + glm::vec3( x, y, z );

		result = glm::length2( com ) < 2;
	}

	return result;

}

glm::vec3 ballFold( float radius, glm::vec3 vec )
{
	/*
	if m<r         m = m/r^2
	  else if m<1 m = 1/m
	*/
	if( glm::length( vec ) < radius )
		return vec / (radius * radius);
	if( glm::length( vec ) < 1 )
		return vec / glm::length2( vec );
	return vec;
}

glm::vec3 boxFold( glm::vec3 vec )
{
	/*
	if v[a]>1          v[a] =  2-v[a]
	  else if v[a]<-1 v[a] =-2-v[a]
	*/
	glm::vec3 result;
	for( int i = 0; i < 3; i++ )
	{
		if ( vec[i] > 1 )
			result[i] = 2 - vec[i];
		else
		if ( vec[i] < -1 )
			result[i] = -2 - vec[i];
	}
	return result;
}

bool mandelboxTestB( float x, float y, float z, float time )
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


	int iterations = 500;
	
	bool result = true;

	glm::vec3 com( 0, 0, 0 );
	float s = 2, r = 0.5, f = 1, bail = 100;

	for( int i = 0; i < iterations && result; i++ )
	{
		com = s*ballFold(r, f*boxFold(com)) + glm::vec3(x,y,z);

		result = glm::length2( com ) < bail;

		if( glm::length2( com ) < 1 )
			return true;
	}

	return result;

}

bool spongeTest( float x, float y, float z, float time )
{
	int iterations = 4;

	bool result = ( x > 0 && x < 1 && y > 0 && y < 1 && z > 0 && z < 1 );

	for( int i = 1; i < iterations && result; i++ )
	{
		float product = std::pow( 3, i );
		int count = 0;

		if (((int)(product * x) % 3)==1)
			count++;
		if (((int)(product * y) % 3)==1)
			count++;
		if (((int)(product * z) % 3)==1)
			count++;

		result = result && count < 2;
	}

	return result;
}

bool depthMandelbrot( float x, float y, float z, float time )
{
	int iterations = 25;
	bool result = z > -1 && z < 1;

	float outputx = 0, outputy = 0;

	for( int i = 0; i < iterations && result; i++ )
	{
		float oldOutputx = outputx;
		outputx = outputx*outputx - outputy*outputy + x;
		outputy = 2 * oldOutputx * outputy + y;

		result = std::sqrt( outputx * outputx + outputy * outputy ) < 1;
	}

	return result;
}

bool imageBusy = false;

glm::vec3 fromOld, toOld;
float aspectOld;
bool needRender = true;

void updateImage( QWidget * widg )
{
	bool needRender = true;
	while( true )
	{
		
		if( image == nullptr || ( image->width() != widg->width() || image->height() != widg->height() ) )
		{
			while( imageBusy )
			{
			}

			imageBusy = true;

			if( image != nullptr )
				delete image;

			image = new QImage(widg->width(),widg->height(),QImage::Format::Format_ARGB32);

			image->fill( QColor( 0, 0, 0 ) );

			imageBusy = false;
			needRender = true;

		}

		

		if( fromOld != cam.from || toOld != cam.to )
		{
			needRender = true;
		}

		if( needRender )
		{
			needRender = false;
			fromOld = cam.from;
			toOld = cam.to;

			if ( !marcher->targetFunction( cam.from.x, cam.from.y, cam.from.z, 0 ) )
			{
				

				//TODO, just pull highest order bit
				for( int iteration = 0; iteration < std::log( image->width() )/ std::log(2) && !( widg->height() != image->height() || widg->width() != image->width() || ( iteration >4 && (fromOld != cam.from || toOld != cam.to ) ) ); iteration++ )
				{
					//for( int i = 0; i < std::pow( 2, iteration ); i++ )
					//{
						int numGroups = std::pow( 2, iteration );

						//Sleep( 500 );

						//std::cout << "iterations: " << std::log( image->width() )/ std::log(2) << " for width of " << image->width() << std::endl;
						//std::cout << "numGroups: " << numGroups << std::endl;

						for( int xGroup = 0; xGroup < numGroups; xGroup ++ )
						{
							for( int yGroup = 0; yGroup < numGroups; yGroup ++ )
							{
								for( int cell = 1; cell < 4 && !( widg->height() != image->height() || widg->width() != image->width() || ( iteration >4 && (fromOld != cam.from || toOld != cam.to ) ) ); cell++ )
								{
									float groupAmount = 1.0f / numGroups;

									float x = ( xGroup + ( (cell%2!=0)? 0.5 : 0 ) ) * groupAmount;
									float y = ( yGroup + ( (cell>1)? 0.5 : 0 ) ) * groupAmount;

									int imageX = x * image->width();
									int imageY = y * image->height();

									//std::cout << "x: " << imageX << " y: " << imageY << std::endl;

									float actualX = 2 * x - 1;
									float actualY = -2 * y + 1;

									float dist;
									glm::vec3 normal;

									bool hit = marcher->marchRay( actualX, actualY, &dist, &normal );

									//(1/(dist*dist+1)) *

									int color = ( ( hit ) ? (int) (255 * std::abs( glm::dot( normal, glm::normalize(cam.to - cam.from) ) ) ) : 0 );
									for( int drawX = imageX; drawX <= imageX + groupAmount*image->width()/2 && drawX < image->width(); drawX++ )
									{
										for( int drawY = imageY; drawY <= imageY + groupAmount*image->height()/2 && drawY < image->height(); drawY++ )
										{
											//image->setPixel(drawX,drawY, ( (hit) ? qRgb( (int)( ( normal.x + 1 ) * 127 ), (int)( (normal.y + 1) * 127), (int)( (normal.z + 1) * 127 ) ) : qRgb( 255,0,0 ) ) );
											image->setPixel(drawX,drawY, qRgb( color, color, color ) );
										}
									}
								}
							}	
						}
					//}
				}

				/*for(unsigned int y = 0; y < image->height() && widg->height() == image->height(); y++) 
				{
					for(unsigned int x = 0; x < image->width() && widg->width() == image->width(); x++) 
					{
						float actualX = 2 * x/float(image->width()) - 1;
						float actualY = 2 * y/float(image->height()) - 1;

						float dist;
						if( marcher->marchRay( actualX, actualY, &dist ) )
						{
							int color = (int) 255 * (3/(dist*dist));
							image->setPixel(x,y,qRgb( color, color, color ) );
						}
					}
				}*/
			}
			else
			{
				for( int drawX = 0; drawX < image->width(); drawX++ )
				{
					for( int drawY = 0; drawY < image->height(); drawY++ )
					{
						//image->setPixel(drawX,drawY, ( (hit) ? qRgb( (int)( ( normal.x + 1 ) * 127 ), (int)( (normal.y + 1) * 127), (int)( (normal.z + 1) * 127 ) ) : qRgb( 255,0,0 ) ) );
						image->setPixel(drawX,drawY, qRgb( 255, 0, 0 ) );
					}
				}
			}
		}
		
	}
}

int currentPixel = 1000000;

void drawImage( QWidget * widg, RayMarcherGPU * march )
{
	while( true )
	{
		if( image == nullptr || ( image->width() != widg->width() || image->height() != widg->height() ) )
		{

			//while(imageBusy)
			//{
			//}
			//imageBusy = true;
			imageHold.lock();
			if( image != nullptr )
				delete image;

			image = new QImage(widg->width(),widg->height(),QImage::Format::Format_ARGB32);

			image->fill( QColor( 0, 0, 0 ) );

			cam.aspect = float(widg->width()) / widg->height();

			//imageBusy = false;
			needRender = true;
			imageHold.unlock();
		}

		if( fromOld != cam.from || toOld != cam.to || aspectOld != cam.aspect )
		{
			needRender = true;
		}

		if( needRender )
		{
			needRender = false;
			

			fromOld = cam.from;
			toOld = cam.to;
			aspectOld = cam.aspect;

			
			march->launch();
			currentPixel = 0;
			//march->destroy();
			//march->initialize();


		}

		
		if( currentPixel < image->width() * image->height() )
		{
			//imageBusy = true;
			//imageHold.lock();
			bool writing = true;
			/*for( int drawY = 0; drawY < image->height(); drawY++ )
			{
				for( int drawX = 0; drawX < image->width(); drawX++ )
				{
					if( (drawX + drawY * image->width()) >= currentPixel && writing)
					{
						glm::vec4 pix = 255.0f * march->getPixel(drawX,drawY);
						writing = pix.a > 5;

						if( writing )
						{
							image->setPixel(drawX,drawY, qRgb( (int)pix.x, (int)pix.y, (int)pix.z ) );
							currentPixel++;
						}
					}
				}
			}*/

			for( int draw = 0; draw < image->height() * image->width(); draw++ )
			{
				for( int drawX = 0; drawX < 1
					//march->subdivideSizes[draw].x * image->width()
					; drawX++ )
				{
					for( int drawY = 0; drawY < 1
						//march->subdivideSizes[draw].y * image->height()
						; drawY++ )
					{
						glm::vec4 pix = 255.0f * march->getPixel(draw);
						writing = pix.a > .5;


						if( writing )
						{
							//std::cout << march->subdividePositions[draw].x*image->width() << " " << march->subdividePositions[draw].y*image->height() << std::endl;
							image->setPixel(drawX+(int)(march->subdividePositions[draw].x*image->width()),
								drawY+(int)(march->subdividePositions[draw].y*image->height()),
								qRgb( (int)pix.x, (int)pix.y, (int)pix.z ) );
								//qRgb( (int)(255*draw/float(image->height() * image->width())), (int)(255*draw/float(image->height() * image->width())), (int)(255*draw/float(image->height() * image->width()))) );
							currentPixel++;
						}
					} 

				}
			}
			//imageBusy = false;
			//imageHold.unlock();
		}
		
	}
}

void RayCastHandle::paintEvent( QPaintEvent *e )
{
	QPainter painter(this);
	painter.setPen( QColor(1,1,1));

	if( image != nullptr )
	{
		//while( imageBusy ){}
		//imageBusy = true;
		imageHold.lock();
		{
			painter.drawImage(QPoint(0,0),*image);
			imageHold.unlock();
		}
		//imageBusy = false;
	}
}

RayCastHandle::RayCastHandle()
{
}

void RayCastHandle::init()
{
	//transform->setCoordinateSystem( CoordinateSystem::SCREEN_COORDINATES );

	resize( 512, 512 );

	//marcher = new RayMarcher( spongeTest , &cam );

	cam.setFrom( glm::vec3( 0, 0, 3 ) );
	cam.setTo( glm::vec3( 0, 0 , 0 ) );

	//std::thread t1( updateImage, this );
	//t1.detach();

	gpu = new RayMarcherGPU( 0, &cam, this );
	gpu->initialize();

	//std::thread t1( drawImage, this, gpu );
	//t1.detach();

	//connect( &intTimer, SIGNAL(timeout()), this, SLOT(interval()) );
	//intTimer.start(0);
	//resize( 256, 256 );
	//std::thread t2( repainting, this );
	//t2.detach();

	
}

void RayCastHandle::interval()
{
	float dt = deltaTimer.interval();
	//float dt = timer.interval();
	//DebugMenus::update();
	update( std::min( 1.0f, dt ) );
	//DebugShapes::update( dt );
}



//void RayCastHandle::paint( Graphics* graphics )
//{
//	graphics->setClearColor(0,0.2,0,1);
//	graphics->clear();
//
//	for( int i = 0; i < width(); i++ )
//	{
//	}
//	graphics->fillRect(
//
//	static float off = 0;
//	off += 0.001;
//
//	graphics->setColor( 1,1,1,1 );
//}
//
void RayCastHandle::update( float dt )
{
	//std::cout << aspectOld << " " << image->width() << " " << image->height() << " " << width() << " " << height() << std::endl;

	static long mostRecent = 0;

	struct stat st;
	long lastUpdated;

    
#ifdef _DEBUGGY
	stat("ray_march.cl", &st);
#else
	stat("../ray_march.cl", &st);
#endif

	lastUpdated = st.st_mtime;

	if( lastUpdated > mostRecent )
	{
		Sleep(20);
		mostRecent = lastUpdated;

		//while( imageBusy )
		//{
		//}
		//imageBusy = true;
		imageHold.lock();
		gpu->unload();
		gpu->load();
		needRender = true;
		imageHold.unlock();
		//imageBusy = false;

		
	}

	if( QApplication::activeWindow() != 0 )
	{
		static glm::vec2 mousePosition = glm::vec2();

		glm::vec3 camNorm = glm::normalize( cam.getTo()-cam.getFrom() );
		glm::vec3 strafe = glm::normalize( -glm::cross( glm::vec3(0,1,0), camNorm ) );
		glm::vec3 up = glm::normalize( glm::cross( strafe, camNorm ) );

		glm::mat3 camSpace = glm::mat3( strafe, camNorm, up );

		glm::vec3 movement;
		/*if ( GetAsyncKeyState(VK_SHIFT) )
			movement = glm::vec3( (GetAsyncKeyState('D')?1:0) - (GetAsyncKeyState('A')?1:0), 0, (GetAsyncKeyState('W')?1:0) - (GetAsyncKeyState('S')?1:0) );
		else*/
			movement = glm::vec3( (GetAsyncKeyState('D')?1:0) - (GetAsyncKeyState('A')?1:0), (GetAsyncKeyState('W')?1:0) - (GetAsyncKeyState('S')?1:0),(GetAsyncKeyState('R')?1:0) - (GetAsyncKeyState('F')?1:0) );

			movement += glm::vec3( (GetAsyncKeyState('J')?1:0) - (GetAsyncKeyState('G')?1:0), (GetAsyncKeyState('Y')?1:0) - (GetAsyncKeyState('H')?1:0),(GetAsyncKeyState('I')?1:0) - (GetAsyncKeyState('K')?1:0) )/20.0f;

		glm::vec3 worldMove = camSpace*movement * 1.0f * dt;

		cam.setTo( cam.getTo() + worldMove);
		cam.setFrom( cam.getFrom() + worldMove);
	}

	repaint();
}

glm::vec2 mousePosition = glm::vec2();
bool mouseTracking = false;


void RayCastHandle::mousePressEvent( QMouseEvent * mEvent )
{
	if ( mEvent->button() == Qt::MouseButton::RightButton )
	{

		if( !mouseTracking )
		{
			mousePosition.x = mEvent->x()/float(width());
			mousePosition.y = mEvent->y()/float(height());
		}
		mouseTracking = true; 
	}

}

void RayCastHandle::mouseReleaseEvent( QMouseEvent * mEvent )
{
	if ( mEvent->button() == Qt::MouseButton::RightButton )
	{
		mouseTracking = false;
	}
}

void RayCastHandle::mouseMoveEvent( QMouseEvent * mEvent )
{
	if ( mouseTracking )
	{
		glm::vec2 currentPosition = glm::vec2( mEvent->x()/float(width()), mEvent->y()/float(height()));

		glm::vec2 delta = currentPosition-mousePosition;

		glm::vec3 camNorm = glm::normalize( cam.getTo()-cam.getFrom() );

		glm::vec3 rotUp = glm::cross( glm::vec3(0,1,0), camNorm );
		glm::vec3 rotRight = glm::cross( rotUp, camNorm );


		cam.setTo( cam.getFrom() + glm::rotate( glm::rotate( camNorm, delta.y*100, rotUp ), delta.x*100, rotRight ) );
		//camera.setTo( camera.getFrom() + glm::rotate( camNorm, delta.y*100, rotUp ) );
		//camera.setTo( glm::vec3( mousePosition.x, mousePosition.y, 0 ) );

		mousePosition.x = currentPosition.x;
		mousePosition.y = currentPosition.y;
	}
}