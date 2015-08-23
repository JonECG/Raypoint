#pragma once

#include <QtWidgets\qframe.h>
#include "StructureObject.h"

class InSceneStructureWidget : public QFrame
{
	StructureObject st;
	QWidget * outletList;
	int currentType;
public:
	InSceneStructureWidget( StructureObject obj );

	friend struct SceneModel;
};

