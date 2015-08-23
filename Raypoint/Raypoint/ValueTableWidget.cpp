#include "ValueTableWidget.h"
#include <QtWidgets\qheaderview.h>

ValueTableWidget::ValueTableWidget()
{
	setColumnCount( 2 );
	QStringList list;
	list.append( tr( "Name" ) );
	list.append( tr( "Default" ) );
	setHorizontalHeaderLabels( list );

	setColumnWidth( 0, 65 );
	setColumnWidth( 1, 65 );

	horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
	horizontalHeader()->setSectionsClickable( false );
	horizontalHeader()->setSelectionMode(QAbstractItemView::NoSelection);
	verticalHeader()->setVisible( false );
}


void ValueTableWidget::eraseAll()
{
	while( rowCount() > 0 )
		removeRow(0);
}

void ValueTableWidget::addFullRow()
{
	insertRow(rowCount());

	selectRow(rowCount()-1);
	blockSignals( true );

	QTableWidgetItem * item;
	item = new QTableWidgetItem();
	//connect(
	//item->
	item->setData( Qt::DisplayRole, "Name" );
	setItem( rowCount()-1, 0, item );

	item = new QTableWidgetItem();
	item->setData( Qt::DisplayRole, 0.0 );
	setItem( rowCount()-1, 1, item );

	blockSignals( false );
}

QString ValueTableWidget::getName( int row )
{
	return item( row, 0 )->text();
}

float ValueTableWidget::getValue( int row )
{
	return item( row, 1 )->data(Qt::DisplayRole).toFloat();
}

void ValueTableWidget::setName( int row, const char * name )
{
	item( row, 0 )->setData( Qt::DisplayRole, name );
}

void ValueTableWidget::setValue( int row, float val )
{
	item( row, 1 )->setData( Qt::DisplayRole, val );
}