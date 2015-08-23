#include "ConsoleWidget.h"
#include <QtWidgets\qlayout.h>
#include <ctime>

ConsoleWidget::ConsoleWidget(void)
{
	setLayout( new QVBoxLayout() );

	list = new QListWidget();
	list->setStyleSheet( tr(
		"QListWidget {"
			"background-color: #AAA;"
		"}"
		"QListWidget::item:selected {"
			"background-color: #333;"
		"}"
		));

	layout()->addWidget( list );

	list->setSpacing( 1 );

	list->setWordWrap( true );
}


void ConsoleWidget::log( const char * message, MESSAGE_TYPE type )
{
	char * str = new char[40];
	time_t test = time(0);
	tm timeThingy;
	localtime_s( &timeThingy, &test );
	strftime( str, 40, "%H:%M:%S -- ", &timeThingy );

	QString adjustMessage = QString( message ).length() > 100 ? QString( message ).left( 100 ) + "...<HOVER TO READ MORE>" : message;
	list->addItem( tr( str ) + adjustMessage );
	QListWidgetItem * item = list->item( list->count() - 1 );
	item->setToolTip( message );

	delete str;
	
	

	switch( type )
	{
	case MESSAGE_TYPE::MSG_INFO:
		item->setTextColor( QColor( 255,255,255) );
		item->setBackgroundColor( QColor( 120, 120, 120 ) );
		break;
	case MESSAGE_TYPE::MSG_WARNING:
		item->setTextColor( QColor( 0,0,0) );
		item->setBackgroundColor( QColor( 235, 235, 40 ) );
		break;
	case MESSAGE_TYPE::MSG_ERROR:
		item->setTextColor( QColor( 255,255,255) );
		item->setBackgroundColor( QColor( 215, 20, 20 ) );
		break;
	case MESSAGE_TYPE::MSG_DIREERROR:
		item->setTextColor( QColor( 255,0,0) );
		item->setBackgroundColor( QColor( 100, 0, 0 ) );
		break;
	}
}

void ConsoleWidget::clear()
{
	list->clear();
}
