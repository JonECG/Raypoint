#pragma once
#include <QtWidgets\qlistwidget.h>

class ObjectListWidget : public QListWidget
{
public:
	ObjectListWidget();
	void refreshList();
	void seekObject( QString string );
};

