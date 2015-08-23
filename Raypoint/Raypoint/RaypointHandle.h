#pragma once
#include "QtWidgets\qwidget.h"
#include "QtCore\qtimer.h"
#include <QtGui\qmouseevent>

#include "Timer.h"

class RaypointHandle : public QWidget
{
	Timer deltaTimer;
	
	QTimer intTimer;
	void test();

	
public:
	RaypointHandle();

	void paintEvent( QPaintEvent * pEvent );
	void init();
	void update(float dt);

	void mousePressEvent( QMouseEvent * mEvent );
	void wheelEvent( QWheelEvent * mEvent );
	void mouseMoveEvent( QMouseEvent * mEvent );
	void mouseReleaseEvent( QMouseEvent * mEvent );

private:
	void interval();
};

