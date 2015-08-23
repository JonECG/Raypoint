#pragma once
#include <QtWidgets\qwidget.h>

class IgnoreChildWidget : public QWidget
{
public:
	QSize sizeHint();
};

