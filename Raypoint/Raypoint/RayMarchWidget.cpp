#include "RayMarchWidget.h"

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

void RayMarchWidget::drawImage( RayMarchWidget * widg, RayMarchCL * march )
{
	while( true )
	{
		if( widg->image == nullptr || ( widg->image->width() != widg->width() || widg->image->height() != widg->height() ) )
		{
			widg->imageHold.lock();
			if( widg->image != nullptr )
				delete widg->image;

			widg->image = new QImage(widg->width(),widg->height(),QImage::Format::Format_ARGB32);

			widg->image->fill( QColor( 0, 0, 0 ) );

			widg->cam->aspect = float(widg->width()) / widg->height();

			widg->needRender = true;
			widg->imageHold.unlock();
		}

		/*if( fromOld != cam.from || toOld != cam.to || aspectOld != cam.aspect )
		{
			needRender = true;
		}*/

		if( widg->needRender && widg->ranOnce )
		{			
			if( march->launch( glm::vec2( widg->image->width(), widg->image->height() ) ) )
			{
				widg->needRender = false;
			

				widg->fromOld = widg->cam->from;
				widg->toOld = widg->cam->to;
				widg->aspectOld = widg->cam->aspect;
				widg->currentPixel = 0;
				widg->ranOnce = false;
			}
		}

		Sleep(1);
		
		if( widg->currentPixel < widg->image->width() * widg->image->height() && widg->currentPixel < march->calculatedAmount )//255.0f * march->getPixel(currentPixel).a > 0.5  )
		{
			//imageHold.lock();
			//std::cout << "Begin write" << std::endl;

			while( widg->currentPixel < widg->image->width() * widg->image->height() && widg->currentPixel < march->calculatedAmount )//255.0f * march->getPixel(currentPixel).a > 0.5 )
			{
				glm::vec2 solvedSize = march->subdivideSizes[march->calculatedAmount/2];
				glm::vec2 solvedPosition = march->subdividePositions[widg->currentPixel];
				solvedPosition.x *= widg->image->width();
				solvedPosition.y *= widg->image->height();

				glm::vec4 pix = 255.0f * march->getPixel(widg->currentPixel);

				for( int drawX = 0; drawX < //1
					solvedSize.x * widg->image->width() && solvedPosition.x + drawX < widg->image->width()
					; drawX++ )
				{
					for( int drawY = 0; drawY < //1
						solvedSize.y * widg->image->height() && solvedPosition.y + drawY < widg->image->height()
						; drawY++ )
					{
						
						//writing = pix.a > .5;


						//if( writing )
						//{
							//std::cout << march->subdividePositions[draw].x*image->width() << " " << march->subdividePositions[draw].y*image->height() << std::endl;
							widg->image->setPixel(drawX+(int)(solvedPosition.x), drawY+(int)(solvedPosition.y), qRgb( (int)pix.x, (int)pix.y, (int)pix.z ) );
								//qRgb( (int)(255*draw/float(image->height() * image->width())), (int)(255*draw/float(image->height() * image->width())), (int)(255*draw/float(image->height() * image->width()))) );
							
						//}
					} 

				}
				widg->currentPixel++;
			}

			//std::cout << "End write" << std::endl;
			//imageHold.unlock();
			widg->ranOnce = true;
			widg->needRedraw = true;
		}
		
	}
}

void RayMarchWidget::paintEvent( QPaintEvent *e )
{
	needRedraw = false;
	QPainter painter(this);
	painter.setPen( QColor(1,1,1));

	if( image != nullptr )
	{
		imageHold.lock();
		{
			std::cout << "Draw";
			painter.drawImage(QPoint(0,0),*image);
			imageHold.unlock();
		}
	}
}

RayMarchWidget::RayMarchWidget( int width, int height )
{
	cam = new Camera( 1, 0.1, 10 );
	image = new QImage( width, height,QImage::Format::Format_ARGB32 );
	gpu = new RayMarchCL( 0, cam );

	resize( 256, 256 );

	//init();
}

void RayMarchWidget::reload()
{
	imageHold.lock();
	gpu->unload();
	gpu->load();
	needRender = true;
	imageHold.unlock();
}

void RayMarchWidget::reset()
{
	needRender = true;
	cam->setFrom( glm::dvec3( 0, 0, 3 ) );
	cam->setTo( glm::dvec3( 0, 0 , 0 ) );
}

RayMarchWidget::~RayMarchWidget()
{
	delete cam;
	delete image;
	delete gpu;
}

void RayMarchWidget::init()
{
	currentPixel = 1000000;
	moveAmount = 1;
	mousePosition = glm::vec2();
	mouseTracking = false;

	
	//resize( 512, 512 );
	cam->setFrom( glm::dvec3( 0, 0, 10 ) );
	cam->setTo( glm::dvec3( 0, 0 , 0 ) );

	gpu->initialize();

	std::thread t1( drawImage, this, gpu );
	t1.detach();

	connect( &intTimer, &QTimer::timeout, [this](){ interval(); } );
	//connect( &intTimer, SIGNAL(timeout()), this, interval );
	
	intTimer.start(0);
}

void RayMarchWidget::dirtyRender()
{
	needRender = true;
}

void RayMarchWidget::setCamera( Camera& cam )
{
	this->cam->from = cam.from;
	this->cam->to = cam.to;
	this->cam->up = cam.up;
	this->cam->fov = cam.fov;
	needRender = true;
}

Camera RayMarchWidget::getCamera()
{
	return *cam;
}

void RayMarchWidget::interval()
{
	float dt = deltaTimer.interval();
	update( std::min( 1.0f, dt ) );
}



void RayMarchWidget::update( float dt )
{
//	static long mostRecent = 0;
//
//	struct stat st;
//	long lastUpdated;
//    
//#ifdef _DEBUGGY
//	stat("ray_march.cl", &st);
//#else
//	stat("../ray_march.cl", &st);
//#endif
//
//	lastUpdated = st.st_mtime;
//
//	if( lastUpdated > mostRecent )
//	{
//		Sleep(20);
//		mostRecent = lastUpdated;
//
//		imageHold.lock();
//		gpu->unload();
//		gpu->load();
//		needRender = true;
//		imageHold.unlock();
//		
//	}

	if( QApplication::activeWindow() != 0 && hasFocus()  )
	{
		static glm::vec2 mousePosition = glm::vec2();

		glm::dvec3 camNorm = glm::normalize( cam->getTo()-cam->getFrom() );
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

		cam->setTo( cam->getTo() + worldMove);
		cam->setFrom( cam->getFrom() + worldMove);
	}

	if( fromOld != cam->from || toOld != cam->to || aspectOld != cam->aspect )
	{
		needRender = true;
	}

	if( needRedraw )
		repaint();
}

void RayMarchWidget::wheelEvent(QWheelEvent *event)
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


void RayMarchWidget::mousePressEvent( QMouseEvent * mEvent )
{
	setFocus();

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

void RayMarchWidget::mouseReleaseEvent( QMouseEvent * mEvent )
{
	if ( mEvent->button() == Qt::MouseButton::RightButton )
	{
		mouseTracking = false;
	}
}

void RayMarchWidget::mouseMoveEvent( QMouseEvent * mEvent )
{
	if ( mouseTracking && hasFocus() )
	{
		glm::vec2 currentPosition = glm::vec2( mEvent->x()/float(width()), mEvent->y()/float(height()));

		glm::vec2 delta = currentPosition-mousePosition;

		glm::dvec3 camNorm = glm::normalize( cam->getTo()-cam->getFrom() );

		glm::dvec3 rotUp = glm::cross( glm::dvec3(0,1,0), camNorm );
		glm::dvec3 rotRight = glm::cross( rotUp, camNorm );


		cam->setTo( glm::dvec3(cam->getFrom()) + glm::dvec3(glm::rotate( glm::rotate( camNorm, delta.y*100.0, rotUp ), delta.x*100.0, rotRight ) ) );
		//camera.setTo( camera.getFrom() + glm::rotate( camNorm, delta.y*100, rotUp ) );
		//camera.setTo( glm::vec3( mousePosition.x, mousePosition.y, 0 ) );

		mousePosition.x = currentPosition.x;
		mousePosition.y = currentPosition.y;
	}
}