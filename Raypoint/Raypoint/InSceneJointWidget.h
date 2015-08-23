#pragma once
#include <QtWidgets\qwidget.h>

class InSceneStructureWidget;
class InSceneJointWidget : public QWidget
{
public:
	enum JointType { UNION, INTERSECT, EXCLUDE };

	InSceneStructureWidget * structure;
	InSceneJointWidget *first, *second;
	bool isSingle;
	JointType type;

	InSceneJointWidget( InSceneStructureWidget * structure );
	InSceneJointWidget( InSceneJointWidget * first, InSceneJointWidget * second, JointType type = UNION );
};

