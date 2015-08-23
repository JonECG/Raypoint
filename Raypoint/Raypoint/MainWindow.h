#pragma once
#include "QtWidgets\qwidget.h"

class QTabWidget;
struct SceneModel;

class MainWindow : public QWidget
{
	QTabWidget * tabs;
	SceneModel * scene;

public:
	MainWindow();
};

