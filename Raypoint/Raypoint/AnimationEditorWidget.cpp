#include "AnimationEditorWidget.h"
#include "AnimationGridEditor.h"
#include <QtWidgets\qsplitter.h>
#include <QtWidgets\qlayout.h>
#include <QtWidgets\qlabel.h>
#include <QtWidgets\qformlayout.h>
#include "SceneModel.h"
#include <QtWidgets\qspinbox.h>
#include <QtWidgets\qwidget.h>
#include <functional>
#include "JankConnect.h"
#include "CatmullRom.h"
#include <QtWidgets\qpushbutton.h>
#include <QtCore\qlist.h>
#include "NewRayMarchWidget.h"
#include "NewRayMarchCL.h"
#include <QtWidgets\qscrollarea.h>
#include "Camera.h"
#include <QtWidgets\qcheckbox.h>
#include <QtWidgets\qmessagebox.h>

void AnimationEditorWidget::syncMarcher()
{
	if( march->gpu )
	{
		if( check->isChecked() )
		{
			Camera cam = (*scene)->calcCamera( grid->getCurrentPosition() );
			cam.aspect = ( float(march->width()) / march->height() );
			*(march->gpu->camera) = cam;
		}

		for( unsigned int i = 1; i < (*scene)->variables.size(); i++ )
		{
			march->gpu->setData( i, (*scene)->variables[i].calcValue( 0, grid->getCurrentPosition() ) );
		}

		march->dirtyRender();
	}
}

void AnimationEditorWidget::syncEditor()
{
	QList<QDoubleSpinBox*> listFound = editorPane->findChildren<QDoubleSpinBox*>();
	for( int i = 0; i < listFound.size(); i++ )
	{
		if( !listFound[0]->isEnabled() )
			listFound[i]->setValue( (*this->scene)->variables[grid->highlightedRow].calcValue( i, grid->getCurrentPosition() ) );
	}

	syncMarcher();
}

AnimationEditorWidget::AnimationEditorWidget(SceneModel ** scene)
{
	this->scene = scene;
	this->setLayout( new QVBoxLayout );

	QSplitter * mainArea = new QSplitter( Qt::Vertical );

	//QWidget * containment = new QWidget;
	//containment->setLayout( new QVBoxLayout );
	
	//===containment->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Ignored );

	//QScrollArea * scroll = new QScrollArea();
	//scroll->setWidget( currentStructures );
	//scroll->setWidgetResizable( false );

	grid = new AnimationGridEditor( scene );

	//containment->layout()->addWidget( currentStructures );
	mainArea->addWidget( grid );

	QSplitter * bottomArea = new QSplitter( Qt::Horizontal );

	
	QFrame * editNodeDisplay = new QFrame;
	editorPane = editNodeDisplay;
	editNodeDisplay->setFrameStyle(QFrame::WinPanel | QFrame::Raised);
	editNodeDisplay->setLineWidth( 3 );
	editNodeDisplay->setMidLineWidth( 3 );

	QWidget * hold = new QWidget;
	hold->setLayout( new QVBoxLayout );
	hold->layout()->addWidget( editNodeDisplay );

	QScrollArea * scroll = new QScrollArea;
	scroll->setWidget( hold );
	scroll->setWidgetResizable( true );


	grid->selectionChanged = [this, editNodeDisplay]( int selectedRow, int selectedNode, float time ){
		qDebug( "wooo" );
			
		updateButt->setEnabled( !check->isChecked() && selectedRow == 0 && selectedNode != -1 );

		if ( editNodeDisplay->layout() != NULL )
		{
			QLayoutItem* item;
			while ( ( item = editNodeDisplay->layout()->takeAt( 0 ) ) != NULL )
			{
				delete item->widget();
				delete item;
			}
			delete editNodeDisplay->layout();
		}

		if( selectedRow != -1 )
		{
			SceneVariable * var = &((*this->scene)->variables[selectedRow]);

			editNodeDisplay->setLayout( new QVBoxLayout );

			QWidget * inner = new QWidget;
			editNodeDisplay->layout()->addWidget( inner );

			QFormLayout * lyout = new QFormLayout;
			inner->setLayout( lyout );

			QLabel * varTitle = new QLabel( var->name.c_str() );
			QFont f = varTitle->font();
			f.setPointSize( 12 );
			varTitle->setFont( f );
			lyout->addWidget( varTitle );

			if( selectedNode != -1 )
			{
				//At node
				for( int i = 0; i < var->numUsed; i++ )
				{
					QDoubleSpinBox * dSpin = new QDoubleSpinBox();
					dSpin->setMinimum( -1000 );
					dSpin->setValue( var->animation[selectedNode].values[i] );

					JankConnect::connect( dSpin, SIGNAL(valueChanged(double)), [this, var, i, dSpin, selectedRow, selectedNode ](){
						qDebug( "I edited" );
						var->animation[selectedNode].values[i] = dSpin->value();
						syncMarcher();
					});

					lyout->addRow( new QLabel( var->valueNames[i] ), dSpin );
				}
			}
			else
			{
				//Interpolate
				for( int sub = 0; sub < var->numUsed; sub++ )
				{
					QDoubleSpinBox * dSpin = new QDoubleSpinBox();
					dSpin->setMinimum( -1000 );
					dSpin->setEnabled( false );

					dSpin->setValue( var->calcValue( sub, time ) );

					lyout->addRow( new QLabel( var->valueNames[sub] ), dSpin );
				}
			}
		}
	};

	grid->markerChanged = [this, editNodeDisplay]( float markerPosition ){
		qDebug( "marked" );
		syncEditor();
	};


	QWidget * previewContainer = new QWidget;
	previewContainer->setLayout( new QVBoxLayout );

	march = new NewRayMarchWidget( 256, 256 );
		NewRayMarchCL * under = new NewRayMarchCL();
		under->initialize();
		march->gpu = under;
		march->init();

	QWidget * toolsBar = new QWidget;
	toolsBar->setLayout( new QHBoxLayout );

	check = new QCheckBox( "Auto" );
	check->setChecked( true );
	connect( check, &QCheckBox::clicked, [this](){ updateButt->setEnabled( !check->isChecked() && grid->getCurrentSelectedRow() == 0 && grid->getCurrentSelectedNode() != -1 ); syncMarcher(); } );

	updateButt = new QPushButton( "Update Node" );
	updateButt->setEnabled( false );
	connect( updateButt, &QPushButton::clicked, [this](){
		if( grid->getCurrentSelectedNode() != -1 )
		{
			Camera input = *(march->gpu->camera);

			KeyPoint * var = &((*this->scene)->variables[0].animation[ grid->getCurrentSelectedNode() ]);
			var->values[0] = input.from.x;
			var->values[1] = input.from.y;
			var->values[2] = input.from.z;
			var->values[3] = input.to.x;
			var->values[4] = input.to.y;
			var->values[5] = input.to.z;
			var->values[6] = input.up.x;
			var->values[7] = input.up.y;
			var->values[8] = input.up.z;
			var->values[9] = input.fov;

			syncEditor();
		} 
	});

	QPushButton * loadButt = new QPushButton( "Load" );
	connect( loadButt, &QPushButton::clicked, [this](){
		try
		{
			march->load( "Assets/temp/testAll.cl" );
		}
		catch( char * exception )
		{
			exception;
			QMessageBox mbox;
			mbox.setWindowTitle( "An error has occurred" );
			mbox.setText( "The scene cannot be compiled, please switch to the Scene Editor tab and diagnose the issue to continue" );
			mbox.exec();
		}
	});

	previewContainer->layout()->addWidget( march );

	toolsBar->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed );
	toolsBar->layout()->setAlignment( Qt::AlignHCenter );
	toolsBar->layout()->addWidget( check );
	toolsBar->layout()->addWidget( updateButt );
	toolsBar->layout()->addWidget( loadButt );

	previewContainer->layout()->addWidget( toolsBar );

	bottomArea->addWidget( previewContainer );
	bottomArea->addWidget( scroll );

	

	mainArea->addWidget( bottomArea );

	this->layout()->addWidget( mainArea );
}
