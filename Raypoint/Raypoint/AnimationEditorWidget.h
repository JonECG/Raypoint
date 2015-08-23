#pragma once
#include <QtWidgets\qwidget.h>
class AnimationGridEditor;
class QFormLayout;
class NewRayMarchWidget;
class QCheckBox;
class QPushButton;
struct SceneModel;

class AnimationEditorWidget : public QWidget
{
	AnimationGridEditor * grid;
	QWidget * editorPane;
	NewRayMarchWidget * march;
	SceneModel ** scene;

	QCheckBox * check;
	QPushButton * updateButt;

	void syncMarcher();
	void syncEditor();
public:
	AnimationEditorWidget(SceneModel ** scene);
};

