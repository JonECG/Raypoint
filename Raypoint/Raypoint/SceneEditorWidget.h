#pragma once
#include <QtWidgets\qwidget.h>

class ObjectListWidget;
class ValueTableWidget;
struct SceneModel;
class ConsoleWidget;

class SceneEditorWidget : public QWidget
{
	ObjectListWidget * objectList;
	QWidget * currentStructures;
	ValueTableWidget * sceneVariables;
	ConsoleWidget * console;
	SceneModel ** scene;

	void keyPressEvent( QKeyEvent *e );
public:
	SceneEditorWidget( SceneModel ** scene );
};

