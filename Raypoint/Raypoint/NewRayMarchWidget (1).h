#pragma once

#include "QtWidgets\qwidget.h"
#include "QtCore\qtimer.h"
#include <QtGui\qmouseevent>

#include "Timer.h"
#include "glm\glm.hpp"
#include <mutex>

class NewRayMarchCL;
class QImage;

class NewRayMarchWidget : public QWidget
{

	QImage * image;
	std::mutex imageHold;

	bool ranOnce;
	bool imageBusy;

	glm::dvec3 fromOld, toOld;
	float aspectOld;
	bool needRender;

	double moveAmount;
	bool mouseTracking;
	glm::vec2 mousePosition;

	int currentPixel;

	Timer deltaTimer;
	
	QTimer intTimer;
	void test();

	static void drawImage( NewRayMarchWidget * widg );
public:
	NewRayMarchCL * gpu;

	NewRayMarchWidget( int width, int height );
	~NewRayMarchWidget();

	bool needRedraw;
	bool canMove;
	void dirtyRender();

	void paintEvent( QPaintEvent * pEvent );
	void init();
	void reset();
	void load( const char * filename );
	void update(float dt);

	void mousePressEvent( QMouseEvent * mEvent );
	void wheelEvent( QWheelEvent * mEvent );
	void mouseMoveEvent( QMouseEvent * mEvent );
	void mouseReleaseEvent( QMouseEvent * mEvent );

private:
	void interval();
};

