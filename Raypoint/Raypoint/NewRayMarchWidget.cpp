#include "NewRayMarchWidget.h"

#include <functional>

#include <iostream>
#include "Camera.h"

#include "glm\glm.hpp"
#include "NewRayMarchCL.h"

#include "QtGui\qpainter.h"
#include "QtGui\qimage.h"
#include <thread>

#include <sys/types.h>
#include <sys/stat.h> 

#include <Windows.h>
#include <QtWidgets\qapplication.h>
#include <mutex>

void NewRayMarchWidget::drawImage( NewRayMarchWidget * widg )
{
	while( true )
	{
		if( widg->gpu && ( widg->image == nullptr || ( widg->image->width() != widg->width() || widg->image->height() != widg->height() ) ) )
		{
			widg->imageHold.lock();
			if( widg->image != nullptr )
				delete widg->image;

			widg->image = new QImage(widg->width(),widg->height(),QImage::Format::Format_ARGB32);

			widg->image->fill( QColor( 0, 0, 0 ) );

			widg->gpu->camera->aspect = float(widg->width()) / widg->height();

			widg->needRender = true;
			widg->imageHold.unlock();
		}

		if( widg->gpu && widg->needRender && widg->ranOnce )
		{			
			if( widg->gpu->launch( glm::vec2( widg->image->width(), widg->image->height() ) ) )
			{
				widg->needRender = false;
			

				widg->fromOld = widg->gpu->camera->from;
				widg->toOld = widg->gpu->camera->to;
				widg->aspectOld = widg->gpu->camera->aspect;
				widg->currentPixel = 0;
				widg->ranOnce = false;
			}
		}

		Sleep(1);
		
		if( widg->gpu && widg->currentPixel < widg->image->width() * widg->image->height() && widg->currentPixel < widg->gpu->calculatedAmount )//255.0f * march->getPixel(currentPixel).a > 0.5  )
		{
			//imageHold.lock();
			//std::cout << "Begin write" << std::endl;

			while( widg->currentPixel < widg->image->width() * widg->image->height() && widg->currentPixel < widg->gpu->calculatedAmount )//255.0f * march->getPixel(currentPixel).a > 0.5 )
			{
				glm::vec2 solvedSize = widg->gpu->subdivideSizes[std::max( widg->gpu->calculatedAmount/2, widg->currentPixel )];
				glm::vec2 solvedPosition = widg->gpu->subdividePositions[widg->currentPixel];
				int posXInt = std::ceil( solvedPosition.x * widg->image->width() - 0.5 );
				int posYInt = std::ceil( solvedPosition.y * widg->image->height() - 0.5 );

				glm::vec4 pix = 255.0f * widg->gpu->getPixel(widg->currentPixel);

				for( int drawX = 0; drawX < solvedSize.x * widg->image->width() && posXInt + drawX < widg->image->width(); drawX++ )
				{
					for( int drawY = 0; drawY < solvedSize.y * widg->image->height() && posYInt + drawY < widg->image->height(); drawY++ )
					{
						widg->image->setPixel(drawX+posXInt, drawY+posYInt, qRgb( (int)pix.x, (int)pix.y, (int)pix.z ) );
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

void NewRayMarchWidget::paintEvent( QPaintEvent *e )
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

NewRayMarchWidget::NewRayMarchWidget( int width, int height )
{
	canMove = true;
	image = new QImage( width, height,QImage::Format::Format_ARGB32 );
	gpu = 0;//new RayMarchCL( 0, cam );

	resize( 256, 256 );
}

void NewRayMarchWidget::load( const char * filename )
{
	imageHold.lock();
	try
	{
		gpu->load( filename );
		needRender = true;
	}
	catch( char * except )
	{
		imageHold.unlock();
		throw except;
	}
	imageHold.unlock();
}

void NewRayMarchWidget::reset()
{
	if( gpu )
	{
		needRender = true;
		gpu->camera->setFrom( glm::dvec3( 0, 0, 3 ) );
		gpu->camera->setTo( glm::dvec3( 0, 0 , 0 ) );
	}
}

NewRayMarchWidget::~NewRayMarchWidget()
{
	delete image;
	if( gpu )
		delete gpu;
}

void NewRayMarchWidget::init()
{
	currentPixel = 1000000;
	moveAmount = 1;
	mousePosition = glm::vec2();
	mouseTracking = false;

	std::thread t1( drawImage, this );
	t1.detach();

	connect( &intTimer, &QTimer::timeout, [this](){ interval(); } );
	//connect( &intTimer, SIGNAL(timeout()), this, interval );
	
	intTimer.start(10);
}

void NewRayMarchWidget::dirtyRender()
{
	needRender = true;
}

void NewRayMarchWidget::interval()
{
	float dt = deltaTimer.interval();
	update( std::min( 1.0f, dt ) );
}



void NewRayMarchWidget::update( float dt )
{
	if( gpu && canMove && QApplication::activeWindow() != 0 && hasFocus()  )
	{
		static glm::vec2 mousePosition = glm::vec2();

		glm::dvec3 camNorm = glm::normalize( gpu->camera->getTo()-gpu->camera->getFrom() );
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

		gpu->camera->setTo( gpu->camera->getTo() + worldMove);
		gpu->camera->setFrom( gpu->camera->getFrom() + worldMove);
	}

	if( gpu && ( canMove && fromOld != gpu->camera->from || toOld != gpu->camera->to || aspectOld != gpu->camera->aspect ) )
	{
		needRender = true;
	}

	if( needRedraw )
		repaint();
}

void NewRayMarchWidget::wheelEvent(QWheelEvent *event)
{
	if( canMove )
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
}


void NewRayMarchWidget::mousePressEvent( QMouseEvent * mEvent )
{
	setFocus();

	if( canMove && mEvent->button() == Qt::MouseButton::RightButton )
	{

		if( !mouseTracking )
		{
			mousePosition.x = mEvent->x()/float(width());
			mousePosition.y = mEvent->y()/float(height());
		}
		mouseTracking = true; 
	}

}

void NewRayMarchWidget::mouseReleaseEvent( QMouseEvent * mEvent )
{
	if ( mEvent->button() == Qt::MouseButton::RightButton )
	{
		mouseTracking = false;
	}
}

void NewRayMarchWidget::mouseMoveEvent( QMouseEvent * mEvent )
{
	if ( gpu && canMove && mouseTracking && hasFocus() )
	{
		glm::vec2 currentPosition = glm::vec2( mEvent->x()/float(width()), mEvent->y()/float(height()));

		glm::vec2 delta = currentPosition-mousePosition;

		glm::dvec3 camNorm = glm::normalize( gpu->camera->getTo()-gpu->camera->getFrom() );

		glm::dvec3 rotUp = glm::cross( glm::dvec3(0,1,0), camNorm );
		glm::dvec3 rotRight = glm::cross( rotUp, camNorm );


		gpu->camera->setTo( glm::dvec3(gpu->camera->getFrom()) + glm::dvec3(glm::rotate( glm::rotate( camNorm, delta.y*100.0, rotUp ), delta.x*100.0, rotRight ) ) );
		//camera.setTo( camera.getFrom() + glm::rotate( camNorm, delta.y*100, rotUp ) );
		//camera.setTo( glm::vec3( mousePosition.x, mousePosition.y, 0 ) );

		mousePosition.x = currentPosition.x;
		mousePosition.y = currentPosition.y;
	}
}