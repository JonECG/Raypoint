#pragma once
#include <QtWidgets\qtablewidget.h>
class ValueTableWidget : public QTableWidget
{
public:
	ValueTableWidget();
	void eraseAll();
	void addFullRow();
	QString getName( int row );
	float getValue( int row );
	void setName( int row, const char * name );
	void setValue( int row, float val );
};

