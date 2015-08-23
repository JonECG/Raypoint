#include "InSceneStructureWidget.h"
#include <QtWidgets\qlayout.h>
#include <QtWidgets\qlabel.h>
#include "CollapsableWidget.h"
#include <QtWidgets\qspinbox.h>
#include <QtWidgets\qcombobox.h>
#include "JankConnect.h"
#include <string>
#include <QtWidgets\qcheckbox.h>
#include <QtWidgets\qlineedit.h>

InSceneStructureWidget::InSceneStructureWidget( StructureObject obj )
{
	currentType = -1;
	st = obj;

	setFrameStyle(QFrame::Box | QFrame::Raised);
	setLineWidth(4);

	this->setLayout( new QVBoxLayout );

	QLabel * name = new QLabel( obj.name.c_str() );
	QFont f = name->font();
	f.setPointSize( 24 );
	name->setFont( f );
	this->layout()->addWidget( name );

	int count = 0;
	int observed = -1;
	for( int i = 0; i < 3; i++ )
	{
		if( obj.enabledModes[i] )
		{
			count++;
			observed = i;
		}
	}

	const char * types[] = { "Subset Raymarch", "Distance Raymarch", "Raytrace" };
	if( count == 1 )
	{
		QLabel * typeLabel = new QLabel( types[observed] );
		this->layout()->addWidget( typeLabel );
	}
	if( count > 1 )
	{
		QComboBox * combo = new QComboBox();

		for( int i = 0; i < 3; i++ )
		{
			if( obj.enabledModes[i] )
			{
				combo->addItem( types[i] );
			}
		}
		
		JankConnect::connect( combo, SIGNAL(currentIndexChanged(QString)), [this,combo](){
			int ind = combo->currentIndex();
			int resulting = 0;
			while( ind >= 0 )
			{
				if( st.enabledModes[resulting] )
				{
					ind--;
				}
				resulting++;
			}
			currentType = resulting - 1;
			//qDebug( std::to_string( currentType ).c_str() );
		});
			
		this->layout()->addWidget( combo );
	}

	currentType = observed;


	CollapsableWidget * outletCollapse = new CollapsableWidget;

	outletCollapse->setTitle( "Outlets" );

	if( obj.numOutlets > 0 )
	{
		outletList = new QWidget;
		QGridLayout * layt = new QGridLayout;
		outletList->setLayout( layt );
		//outletList->setContentsMargins(0,0,0,0);

		for( int i = 0; i < obj.numOutlets; i++ )
		{
			layt->addWidget( new QLabel( obj.outletNames[i].c_str() ), i, 0 );
			QCheckBox * check = new QCheckBox( "Use Default" );
			QLineEdit * outEdit = new QLineEdit();
			connect( check, &QCheckBox::stateChanged, [this,i,check,outEdit](){
				if( check->isChecked() )
					outEdit->setText( QString::number( this->st.outletValues[i] ) );
				outEdit->setEnabled( !check->isChecked() );
			});
			check->setChecked( true );
			layt->addWidget( outEdit, i, 1 );
			layt->addWidget( check, i, 2 );
		}

		outletCollapse->setWidget( outletList );

		this->layout()->addWidget( outletCollapse );
	}

	setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
	setAutoFillBackground( true );
	setBackgroundRole( QPalette::Button );
	//layout()->setWidt
	//layout()->setSizeConstraint(QLayout::Set
}