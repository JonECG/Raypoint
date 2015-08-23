#pragma once

#include "QtWidgets\qwidget.h"
#include "QtCore\qtimer.h"
#include <QtGui\qmouseevent>

#include "Timer.h"

#include "glm\glm.hpp"

#include <mutex>

class QImage;
class RayMarchCL;
class Camera;

class RayMarchWidget : public QWidget
{
	Camera * cam;

	QImage * image;
	RayMarchCL * gpu;
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

	static void drawImage( RayMarchWidget * widg, RayMarchCL * march );
public:
	RayMarchWidget( int width, int height );
	~RayMarchWidget();

	bool needRedraw;
	void dirtyRender();
	void setCamera(Camera& cam);
	Camera getCamera();

	void paintEvent( QPaintEvent * pEvent );
	void init();
	void reset();
	void reload();
	void update(float dt);

	void mousePressEvent( QMouseEvent * mEvent );
	void wheelEvent( QWheelEvent * mEvent );
	void mouseMoveEvent( QMouseEvent * mEvent );
	void mouseReleaseEvent( QMouseEvent * mEvent );

private:
	void interval();
};

