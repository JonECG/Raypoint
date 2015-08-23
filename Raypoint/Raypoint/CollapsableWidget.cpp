#include "CollapsableWidget.h"
#include <QtWidgets\qlayout.h>
#include <QtWidgets\qpushbutton.h>
#include <QtWidgets\qframe.h>

CollapsableWidget::CollapsableWidget()
{
	isCollapsed = true;

	this->setLayout( new QVBoxLayout() );
	this->layout()->setSpacing(0);

	button = new QPushButton( "Button" );
	//button->setContentsMargins(5,5,5,0);
	frame = new QFrame();
	frame->setLayout( new QVBoxLayout );
	frame->setFrameStyle(QFrame::Panel | QFrame::Raised);
	frame->setLineWidth(2);
	frame->setVisible( false );
	//frame->setContentsMargins(5,0,5,5);
	frame->setAutoFillBackground( true );
	frame->setBackgroundRole( QPalette::Background );

	connect( button, &QPushButton::clicked, [this](){ isCollapsed = !isCollapsed; frame->setVisible( !isCollapsed ); button->setText( (isCollapsed) ? "View " + title : "Hide " + title ); } );

	this->layout()->addWidget( button );
	this->layout()->addWidget( frame );
}

void CollapsableWidget::setTitle( QString s )
{
	title = s;
	button->setText( (isCollapsed) ? "View " + s : "Hide " + s );
}

void CollapsableWidget::setWidget( QWidget * w )
{
	frame->layout()->addWidget( w );
}
