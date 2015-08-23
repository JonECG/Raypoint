#include "SceneEditorWidget.h"
#include <QtWidgets\qlayout.h>
#include <QtWidgets\qlabel.h>
#include "ObjectListWidget.h"
#include <QtWidgets\qpushbutton.h>
#include "ValueTableWidget.h"
#include <QtWidgets\qmessagebox.h>
#include <QtWidgets\qsplitter.h>
#include "InSceneStructureWidget.h"
#include "StructureObject.h"
#include <QtWidgets\qscrollarea.h>
#include "IgnoreChildWidget.h"
#include <iostream>
#include "SceneModel.h"
#include "JankConnect.h"
#include "NewRayMarchWidget.h"
#include "NewRayMarchCL.h"
#include <QtWidgets\qapplication.h>
#include <QtWidgets\qfiledialog.h>
#include "ConsoleWidget.h"

SceneEditorWidget::SceneEditorWidget(SceneModel ** scene)
{
	this->scene = scene;
	////////////Object List/////////////////////////////
	this->setLayout( new QHBoxLayout() );

	QWidget * objectListContainer = new QWidget;
	objectListContainer->setLayout( new QVBoxLayout );

	objectListContainer->layout()->addWidget( new QLabel( "Objects" ) );

	objectList = new ObjectListWidget();
	objectList->refreshList();

	objectListContainer->setFixedWidth( 150 );

	objectListContainer->layout()->addWidget( objectList );

	QPushButton * addToScene = new QPushButton( "Add" );
	connect( addToScene, &QPushButton::clicked, [this](){
		if( objectList->currentRow() != -1 )
		{
			StructureObject sto = StructureObject::load( objectList->item( objectList->currentRow() )->text().toStdString() );
			InSceneStructureWidget * widg = new InSceneStructureWidget( sto );
			(*this->scene)->structures.push_back( widg );
			//currentStructures->layout()->addWidget( new QLabel( "TESTING" ) );
			currentStructures->layout()->addWidget( widg );
			//std::cout << currentStructures->layout()->children().count() << std::endl;
		}
	});

	objectListContainer->layout()->addWidget( addToScene );

	this->layout()->addWidget( objectListContainer );

	//////////////Main Area//////////////////////////////


	QSplitter * mainArea = new QSplitter( Qt::Vertical );

	QWidget * containment = new QWidget;
	containment->setLayout( new QVBoxLayout );

	currentStructures = new QWidget;
	currentStructures->setLayout( new QVBoxLayout );
	currentStructures->layout()->setAlignment( Qt::AlignTop );
	currentStructures->layout()->setSpacing( 20 );

	QScrollArea * scroll = new QScrollArea();
	scroll->setWidget( currentStructures );
	containment->layout()->addWidget( scroll );
	scroll->setWidgetResizable( true );
	scroll->setBackgroundRole( QPalette::Shadow );

	mainArea->addWidget( containment );

	///////
	QSplitter * horizontalBottomBar = new QSplitter( Qt::Horizontal );
	
	horizontalBottomBar->setFixedHeight( 256 );

	QWidget * previewVerticalContainer = new QWidget;
	previewVerticalContainer->setLayout( new QVBoxLayout );

	QLabel * prevLabel = new QLabel( "Preview" );
	prevLabel->setFixedHeight( 24 );
	previewVerticalContainer->layout()->addWidget( prevLabel ); 
	NewRayMarchWidget * march = new NewRayMarchWidget( 256, 256 );
		NewRayMarchCL * under = new NewRayMarchCL();
		under->initialize();
		march->gpu = under;
	
	previewVerticalContainer->layout()->addWidget( march ); //preview
	march->init();

	//previewVerticalContainer->setFixedWidth( 256 );

	QWidget * previewButtonBar = new QWidget;
	previewButtonBar->setFixedHeight( 38 );
	previewButtonBar->setLayout( new QHBoxLayout );
	previewButtonBar->layout()->setAlignment( Qt::AlignHCenter );

	QPushButton * compButt = new QPushButton( "Preview Scene" );
	connect( compButt, &QPushButton::clicked, [this,march](){
		if( (*this->scene)->structures.size() == 0 )
		{
			console->log( "Scene is empty", ConsoleWidget::MESSAGE_TYPE::MSG_ERROR );
		}
		else
		{
			console->log( "Compiling scene" );
			try
			{
				(*this->scene)->visualize( march, true );
			}
			catch ( char * error )
			{
				console->log( ( "UNEXPECTED COMPILATION ERROR: " + std::string( error ) ).c_str(), ConsoleWidget::MESSAGE_TYPE::MSG_DIREERROR );
			}
			console->log( "Compiling ended" );
		}
	});

	previewButtonBar->layout()->addWidget( compButt );
	QPushButton * viewButt = new QPushButton( "Reset View" );
	connect( viewButt, &QPushButton::clicked, [march](){ march->reset(); } );
	previewButtonBar->layout()->addWidget( viewButt );

	previewVerticalContainer->layout()->addWidget( previewButtonBar ); //preview

	horizontalBottomBar->addWidget( previewVerticalContainer );

	QWidget * consoleVerticalContainer = new QWidget;
	consoleVerticalContainer->setLayout( new QVBoxLayout );
	consoleVerticalContainer->setMinimumWidth( 256 );

	consoleVerticalContainer->layout()->addWidget( new QLabel( "Console" ) ); 

	console = new ConsoleWidget;

	consoleVerticalContainer->layout()->addWidget( console ); 

	horizontalBottomBar->addWidget( consoleVerticalContainer );
	///////

	mainArea->addWidget( horizontalBottomBar );

	this->layout()->addWidget( mainArea );


	/////////////Scene Variables/////////////////////////

	QWidget * sceneVariablesContainer = new QWidget;
	sceneVariablesContainer->setLayout( new QVBoxLayout );

	sceneVariablesContainer->layout()->addWidget( new QLabel( "Scene Variables" ) );

	sceneVariables = new ValueTableWidget();

	sceneVariablesContainer->setFixedWidth( 150 );

	sceneVariablesContainer->layout()->addWidget( sceneVariables );

	JankConnect::connect( sceneVariables, SIGNAL(itemChanged( QTableWidgetItem* )), [this](){
		qDebug( "Value changed" );
		(*this->scene)->variables[sceneVariables->currentRow()+1].name = sceneVariables->getName(sceneVariables->currentRow()).toStdString();
		(*this->scene)->variables[sceneVariables->currentRow()+1].defaultValues[0] = sceneVariables->getValue(sceneVariables->currentRow());
		//(*this->scene)->variables[selectedRow].animation[selectedNode].value = dSpin->value();
	});

	QPushButton * addButt = new QPushButton( "Add" );
	connect( addButt, &QPushButton::clicked, [this](){
		SceneVariable sc( "Name" );
		sc.addValue( "Value", 0 );
		(*this->scene)->variables.push_back( sc );
		sceneVariables->addFullRow();
		//sceneVariables->set
		//hasChanged = true;
	} );
	sceneVariablesContainer->layout()->addWidget( addButt );

	QPushButton * deleteButt = new QPushButton( "Delete" );
	connect( deleteButt, &QPushButton::clicked, [this](){
		if( sceneVariables->currentRow() != -1 )
		{
			QMessageBox msgBox;
			msgBox.setWindowTitle("Deleting scene variable");
			msgBox.setText("Are you sure you want to delete the scene variable \"" + sceneVariables->getName( sceneVariables->currentRow() ) + "\"?");
			msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
			msgBox.setDefaultButton(QMessageBox::No);
			int result = msgBox.exec();
			if( result == QMessageBox::Yes )
			{
				(*this->scene)->variables.erase( (*this->scene)->variables.begin() + sceneVariables->currentRow() + 1 );
				sceneVariables->removeRow( sceneVariables->currentRow() );
				//hasChanged = true;
			}
		}
	} );
	sceneVariablesContainer->layout()->addWidget( deleteButt );

	this->layout()->addWidget( sceneVariablesContainer );
}


void SceneEditorWidget::keyPressEvent( QKeyEvent *e )
{
	if ( (e->key() == Qt::Key_S) && QApplication::keyboardModifiers() && Qt::ControlModifier)
    {
		if( !(*scene)->autoSave || ( QApplication::keyboardModifiers() && Qt::ShiftModifier ) )
		{
			QString fileName = QFileDialog::getSaveFileName(this,tr("Save Scene"), "", tr("Raypoint Scene (*.rpscn)"));
			if( !fileName.isEmpty() )
			{
				(*scene)->save( fileName.toStdString() );
				console->log( ( "File successfully saved to " + fileName ).toStdString().c_str() );
			}
		}
	}

	if ( (e->key() == Qt::Key_O) && QApplication::keyboardModifiers() && Qt::ControlModifier)
    {
		QString fileName = QFileDialog::getOpenFileName(this,tr("Load Scene"), "", tr("Raypoint Scene (*.rpscn)"));
		if( !fileName.isEmpty() )
		{
			(**scene) = SceneModel::load( fileName.toStdString() );

			while( sceneVariables->rowCount() > 0 )
				sceneVariables->removeRow( 0 );

			for( unsigned int i = 1; i < (*scene)->variables.size(); i++ )
			{
				sceneVariables->addFullRow();
				sceneVariables->setName( i - 1, (*scene)->variables[i].name.c_str() );
				sceneVariables->setValue( i - 1, (*scene)->variables[i].defaultValues[0] );
			}


			QLayoutItem* item;
			while ( ( item = currentStructures->layout()->takeAt( 0 ) ) != NULL )
			{
				delete item->widget();
				delete item;
			}
			//delete currentStructures->layout();
			//currentStructures->setLayout( 
			for( unsigned int i = 0; i < (*scene)->structures.size(); i++ )
			{
				currentStructures->layout()->addWidget( (*scene)->structures[i] );
			}

			console->log( ( "File loaded " + fileName ).toStdString().c_str() );
		}
	}
}