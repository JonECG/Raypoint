#include "InSceneJointWidget.h"

InSceneJointWidget::InSceneJointWidget( InSceneStructureWidget * structure )
{
	this->structure = structure;
	isSingle = true;
}

InSceneJointWidget::InSceneJointWidget( InSceneJointWidget * first, InSceneJointWidget * second, JointType type )
{
	this->first = first;
	this->second = second;
	this->type = type;
	isSingle = false;
}