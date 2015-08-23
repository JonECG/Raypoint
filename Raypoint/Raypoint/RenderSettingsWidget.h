#pragma once
#include <QtWidgets\qwidget.h>
#include <QtCore\qtimer.h>
#include "Timer.h"

struct SceneModel;

class RenderSettingsWidget : public QWidget
{
	SceneModel ** scene;
	QString path;
	bool selectedPath, isRendering;
	QTimer qtimer;
	Timer dtTimer;

	QString actionString, frameString, timeString, estString;
	float showProgress;

public:
	RenderSettingsWidget( SceneModel ** scene );
};

