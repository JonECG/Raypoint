#pragma once
#include <QtWidgets\qwidget.h>

#include <functional>
#include <QtCore\qtimer.h>
#include "Timer.h"
struct SceneModel;

class AnimationGridEditor : public QWidget
{
	float timeStart;
	float timeSpan;
	bool mouseTracking;
	float mouseTrackedX;
	int mouseTrackButton;
	bool isPlaying;
	float markerPosition;
	float playPosition;
	SceneModel ** scene;
	QTimer qtimer;
	Timer dtTimer;

	bool isInsideRows( int x, int y, float* time = 0, int* row = 0 );
	float getTime( int pixelX );
	int getRow( int pixelY );
	int timeInPixels( float time );
	int getNodeAt( int x, int y, int row );
public:
	int highlightedRow;
	int highlightedNode;
	std::function< void(int,int,float) > selectionChanged;
	std::function< void(float) > markerChanged;

	AnimationGridEditor(SceneModel ** scene);
	void paintEvent( QPaintEvent * e );
	void mousePressEvent( QMouseEvent * e );
	void mouseMoveEvent( QMouseEvent * e );
	void mouseReleaseEvent( QMouseEvent * e );
	void wheelEvent( QWheelEvent * e );
	bool getIsPlaying();
	float getCurrentPosition();
	int getCurrentSelectedNode();
	int getCurrentSelectedRow();
};

