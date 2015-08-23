#include "RaypointHandle.h"

#include <functional>

#include <iostream>
#include "Camera.h"

#include "glm\glm.hpp"
#include "RayMarchCL.h"

#include "QtGui\qpainter.h"
#include "QtGui\qimage.h"
#include <thread>

#include <sys/types.h>
#include <sys/stat.h> 


#include <Windows.h>
#include <QtWidgets\qapplication.h>
#include <mutex>



Camera cam( 1, 0.1, 10 );

QImage * image;
RayMarchCL * gpu;
std::mutex imageHold;


bool imageBusy = false;

glm::dvec3 fromOld, toOld;
float aspectOld;
bool needRender = true;


int currentPixel = 1000000;

void drawImage( QWidget * widg, RayMarchCL * march )
{
	while( true )
	{
		if( image == nullptr || ( image->width() != widg->width() || image->height() != widg->height() ) )
		{
			imageHold.lock();
			if( image != nullptr )
				delete image;

			image = new QImage(widg->width(),widg->height(),QImage::Format::Format_ARGB32);

			image->fill( QColor( 0, 0, 0 ) );

			cam.aspect = float(widg->width()) / widg->height();

			needRender = true;
			imageHold.unlock();
		}

		/*if( fromOld != cam.from || toOld != cam.to || aspectOld != cam.aspect )
		{
			needRender = true;
		}*/

		if( needRender )
		{			
			if( march->launch( glm::vec2( image->width(), image->height() ) ) )
			{
				needRender = false;
			

				fromOld = cam.from;
				toOld = cam.to;
				aspectOld = cam.aspect;
				currentPixel = 0;
			}
		}

		
		if( currentPixel < image->width() * image->height() && currentPixel < march->calculatedAmount )//255.0f * march->getPixel(currentPixel).a > 0.5  )
		{
			//imageHold.lock();
			//std::cout << "Begin write" << std::endl;

			while( currentPixel < image->width() * image->height() && currentPixel < march->calculatedAmount )//255.0f * march->getPixel(currentPixel).a > 0.5 )
			{
				glm::vec2 solvedSize = march->subdivideSizes[march->calculatedAmount/2];
				glm::vec2 solvedPosition = march->subdividePositions[currentPixel];
				solvedPosition.x *= image->width();
				solvedPosition.y *= image->height();

				for( int drawX = 0; drawX < //1
					solvedSize.x * image->width() && solvedPosition.x + drawX < image->width()
					; drawX++ )
				{
					for( int drawY = 0; drawY < //1
						solvedSize.y * image->height() && solvedPosition.y + drawY < image->height()
						; drawY++ )
					{
						glm::vec4 pix = 255.0f * march->getPixel(currentPixel);
						//writing = pix.a > .5;


						//if( writing )
						//{
							//std::cout << march->subdividePositions[draw].x*image->width() << " " << march->subdividePositions[draw].y*image->height() << std::endl;
							image->setPixel(drawX+(int)(solvedPosition.x), drawY+(int)(solvedPosition.y), qRgb( (int)pix.x, (int)pix.y, (int)pix.z ) );
								//qRgb( (int)(255*draw/float(image->height() * image->width())), (int)(255*draw/float(image->height() * image->width())), (int)(255*draw/float(image->height() * image->width()))) );
							
						//}
					} 

				}
				currentPixel++;
			}

			//std::cout << "End write" << std::endl;
			//imageHold.unlock();
		}
		
	}
}

void RaypointHandle::paintEvent( QPaintEvent *e )
{
	QPainter painter(this);
	painter.setPen( QColor(1,1,1));

	if( image != nullptr )
	{
		imageHold.lock();
		{
			painter.drawImage(QPoint(0,0),*image);
			imageHold.unlock();
		}
	}
}

RaypointHandle::RaypointHandle()
{
}

void RaypointHandle::init()
{
	resize( 512, 512 );
	cam.setFrom( glm::dvec3( 0, 0, 3 ) );
	cam.setTo( glm::dvec3( 0, 0 , 0 ) );

	gpu = new RayMarchCL( 0, &cam );
	gpu->initialize();

	std::thread t1( drawImage, this, gpu );
	t1.detach();

	connect( &intTimer, &QTimer::timeout, [this](){ interval(); } );
	//connect( &intTimer, SIGNAL(timeout()), this, interval );
	
	intTimer.start(0);
}

void RaypointHandle::interval()
{
	float dt = deltaTimer.interval();
	update( std::min( 1.0f, dt ) );
}

double moveAmount = 1;

void RaypointHandle::update( float dt )
{
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

		imageHold.lock();
		gpu->unload();
		gpu->load();
		needRender = true;
		imageHold.unlock();
		
	}

	if( QApplication::activeWindow() != 0 )
	{
		static glm::vec2 mousePosition = glm::vec2();

		glm::dvec3 camNorm = glm::normalize( cam.getTo()-cam.getFrom() );
		glm::dvec3 strafe = glm::normalize( -glm::cross( glm::dvec3(0,1,0), camNorm ) );
		glm::dvec3 up = glm::normalize( glm::cross( strafe, camNorm ) );

		glm::dmat3 camSpace = glm::dmat3( strafe, camNorm, up );

		glm::dvec3 movement;
		/*if ( GetAsyncKeyState(VK_SHIFT) )
			movement = glm::vec3( (GetAsyncKeyState('D')?1:0) - (GetAsyncKeyState('A')?1:0), 0, (GetAsyncKeyState('W')?1:0) - (GetAsyncKeyState('S')?1:0) );
		else*/
			movement = moveAmount*glm::dvec3( (GetAsyncKeyState('D')?1:0) - (GetAsyncKeyState('A')?1:0), (GetAsyncKeyState('W')?1:0) - (GetAsyncKeyState('S')?1:0),(GetAsyncKeyState('R')?1:0) - (GetAsyncKeyState('F')?1:0) );

			movement += glm::vec3( (GetAsyncKeyState('J')?1:0) - (GetAsyncKeyState('G')?1:0), (GetAsyncKeyState('Y')?1:0) - (GetAsyncKeyState('H')?1:0),(GetAsyncKeyState('I')?1:0) - (GetAsyncKeyState('K')?1:0) )/20.0f;

		glm::dvec3 worldMove = camSpace*movement *( 1.0 * dt );

		cam.setTo( cam.getTo() + worldMove);
		cam.setFrom( cam.getFrom() + worldMove);
	}

	if( fromOld != cam.from || toOld != cam.to || aspectOld != cam.aspect )
	{
		needRender = true;
	}

	repaint();
}

void RaypointHandle::wheelEvent(QWheelEvent *event)
 {
     int numDegrees = event->delta() / 8;
     int numSteps = numDegrees / 15;

	 if( event->delta() > 0 )
	 {
		 moveAmount = moveAmount * 5 / 4;
	 }
	 else
	 {
		 moveAmount = moveAmount * 4 / 5;
	 }
     event->accept();
 }

glm::vec2 mousePosition = glm::vec2();
bool mouseTracking = false;


void RaypointHandle::mousePressEvent( QMouseEvent * mEvent )
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

void RaypointHandle::mouseReleaseEvent( QMouseEvent * mEvent )
{
	if ( mEvent->button() == Qt::MouseButton::RightButton )
	{
		mouseTracking = false;
	}
}

void RaypointHandle::mouseMoveEvent( QMouseEvent * mEvent )
{
	if ( mouseTracking )
	{
		glm::vec2 currentPosition = glm::vec2( mEvent->x()/float(width()), mEvent->y()/float(height()));

		glm::vec2 delta = currentPosition-mousePosition;

		glm::dvec3 camNorm = glm::normalize( cam.getTo()-cam.getFrom() );

		glm::dvec3 rotUp = glm::cross( glm::dvec3(0,1,0), camNorm );
		glm::dvec3 rotRight = glm::cross( rotUp, camNorm );


		cam.setTo( glm::dvec3(cam.getFrom()) + glm::dvec3(glm::rotate( glm::rotate( camNorm, delta.y*100.0, rotUp ), delta.x*100.0, rotRight ) ) );
		//camera.setTo( camera.getFrom() + glm::rotate( camNorm, delta.y*100, rotUp ) );
		//camera.setTo( glm::vec3( mousePosition.x, mousePosition.y, 0 ) );

		mousePosition.x = currentPosition.x;
		mousePosition.y = currentPosition.y;
	}
}