#pragma once
#include <QtWidgets\qwidget.h>
#include <QtCore\qstring.h>
class QFrame;
class QPushButton;

class CollapsableWidget : public QWidget
{
	QString title;
	bool isCollapsed;
	QFrame * frame;
	QPushButton * button;

public:
	CollapsableWidget();
	void setTitle( QString s );
	void setWidget( QWidget * w );
};

