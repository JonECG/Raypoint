#pragma once
#include <QtWidgets\qwidget.h>
#include <QtWidgets\qlistwidget.h>

class ConsoleWidget : public QWidget
{
	QListWidget * list;

public:
	enum MESSAGE_TYPE{ MSG_INFO, MSG_WARNING, MSG_ERROR, MSG_DIREERROR };

	ConsoleWidget();

	void log( const char * message, MESSAGE_TYPE type = MSG_INFO );
	void clear();
};

