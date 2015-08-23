#pragma once
#include "Qt\qwidget.h"
#include "Qt\qtimer.h"
#include <QtGui\qmouseevent>

#include "Timer.h"

class RayCastHandle : public QWidget
{
	Q_OBJECT;

	Timer deltaTimer;
	
	QTimer intTimer;
	void test();
public:
	RayCastHandle();

	void paintEvent( QPaintEvent * pEvent );
	void init();
	void update(float dt);

	void mousePressEvent( QMouseEvent * mEvent );
	void mouseMoveEvent( QMouseEvent * mEvent );
	void mouseReleaseEvent( QMouseEvent * mEvent );

private slots:
	void interval();
};

