#include "ObjectEditorWidget.h"
#include <QtWidgets\qlayout.h>
#include <QtWidgets\qtabwidget.h>
#include <QtWidgets\qtextedit.h>
#include <QtWidgets\qlistwidget.h>
#include <QtGui\qfontdatabase.h>
#include <QtWidgets\qlineedit.h>
#include <QtWidgets\qcheckbox.h>
#include <QtWidgets\qlabel.h>
#include <QtWidgets\qpushbutton.h>
#include "NewRayMarchCL.h"
#include "NewRayMarchWidget.h"
#include "ScriptParser.h"
#include "Block.h"
#include <fstream>
#include "CodeEditWidget.h"
#include <QtWidgets\qtablewidget.h>
#include <QtWidgets\qheaderview.h>
#include <QtWidgets\qapplication.h>
#include "StructureObject.h"
#include <QtWidgets\qmessagebox.h>
#include <iostream>
#include <vector>
#include <QtWidgets\qinputdialog.h>
#include <QtWidgets\qsplitter.h>
#include "ObjectListWidget.h"
#include "ValueTableWidget.h"
#include "ConsoleWidget.h"


ObjectEditorWidget::ObjectEditorWidget()
{
	this->setLayout( new QHBoxLayout() );

	QWidget * objectListContainer = new QWidget;
	objectListContainer->setLayout( new QVBoxLayout );

	objectListContainer->layout()->addWidget( new QLabel( "Objects" ) );

	objectList = new ObjectListWidget();
	objectList->refreshList();

	connect( objectList->selectionModel(), &QItemSelectionModel::selectionChanged, [this](){ this->changingSelection(); } );


	objectListContainer->setFixedWidth( 150 );

	objectListContainer->layout()->addWidget( objectList );

	QPushButton * newObjButt = new QPushButton( "New" );
	connect( newObjButt, &QPushButton::clicked, [this](){
		bool ok;
		QString s = QInputDialog::getText(this, tr("New Object"), tr("Name:"), QLineEdit::Normal, tr( "NewObject" ), &ok);
		if (ok && !s.isEmpty())
		{
			StructureObject obj;
			obj.name = s.toStdString();
			display( obj );
			saveCurrent();
			objectList->refreshList();
			hasChanged = false;
			objectList->seekObject( s );
		}
	} );
	objectListContainer->layout()->addWidget( newObjButt );

	QPushButton * deleteObjButt = new QPushButton( "Delete" );
	connect( deleteObjButt, &QPushButton::clicked, [this](){
		if( objectList->currentRow() != -1 )
		{
			QMessageBox msgBox;
			msgBox.setWindowTitle("Deleting Object");
			msgBox.setText("Are you sure you want to delete the object \"" + objectList->item( objectList->currentRow() )->text() + "\"?");
			msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
			msgBox.setDefaultButton(QMessageBox::No);
			int result = msgBox.exec();
			if( result == QMessageBox::Yes )
			{
				remove( ( "Assets/objs/" + ( objectList->item( objectList->currentRow() )->text() + ".rpobj" ).toStdString() ).c_str() );
				editingIndex = -1;
				objectList->selectionModel()->reset();
				objectList->refreshList();				
				hide();
			}
		}
	} );
	objectListContainer->layout()->addWidget( deleteObjButt );

	this->layout()->addWidget( objectListContainer ); //Existing objects widget

	noObjectSelected = new QWidget();
	noObjectSelected->setLayout( new QVBoxLayout() );
	noObjectSelected->layout()->setAlignment( Qt::AlignHCenter );
	noObjectSelected->layout()->addWidget( new QLabel( "No object has been selected for editing" ) );
	QFont nf("Tahoma");
	nf.setPixelSize( 24 );
	noObjectSelected->setFont( nf );
	this->layout()->addWidget( noObjectSelected );

	currentObjectContainer = new QSplitter( Qt::Vertical );
	//currentObjectContainer->setLayout( new QVBoxLayout );

	QWidget * currentObjectEditorContainer = new QWidget;
	currentObjectEditorContainer->setLayout( new QVBoxLayout );

	nameEditor = new QLineEdit;
	QFont lf("Tahoma");
	lf.setPixelSize( 24 );
	nameEditor->setFont(lf);
	nameEditor->setMaximumWidth( 320 );
	connect( nameEditor, &QLineEdit::textEdited, [this](){ hasChanged = true; } );
	currentObjectEditorContainer->layout()->addWidget( nameEditor );

	codeEditTabs = new QTabWidget();

	const char * signatures[] = { "subsetRayMarch( x, y, z, out r, out g, out b ) : boolean", "distanceRayMarch( x, y, z, out r, out g, out b ) : number", "rayTrace( x, y, z, dx, dy, dy, out dist, out r, out g, out b ) : boolean" };

	for( int i = 0; i < 3; i++ )
	{
		QWidget * de = new QWidget;
		de->setLayout( new QVBoxLayout );

	
		QCheckBox * check = new QCheckBox;
		check->setChecked( false );
		check->setText( "Enabled" );
		de->layout()->addWidget( check );

	
		de->layout()->addWidget( new QLabel( signatures[i] ) );

		CodeEditWidget * edit = new CodeEditWidget();
		de->layout()->addWidget( edit );
		edit->setEnabled( false );
		connect( edit, &QTextEdit::textChanged, [this](){ hasChanged = true; } );

		connect( check, &QCheckBox::stateChanged, [edit, check](){ edit->setEnabled( check->isChecked() ); } );
		connect( check, &QCheckBox::clicked, [this](){ hasChanged = true; } );

		codeEditTabs->addTab( de, tr("Test"));
	}

	codeEditTabs->setTabText(0, "Subset Ray Marching" );
	codeEditTabs->setTabText(1, "Distance Estimated Ray Marching" );
	codeEditTabs->setTabText(2, "Absolute Ray Tracing" );

	//QObject::connect(
	//connect( check, SIGNAL( stateChanged( int ) ), [edit](int state){ edit->setEnabled( state == 0 ); } );


	currentObjectEditorContainer->layout()->addWidget( codeEditTabs ); 

	QSplitter * horizontalBottomBar = new QSplitter( Qt::Horizontal );
	//horizontalBottomBar->setLayout( new QHBoxLayout );

	currentObjectContainer->addWidget( currentObjectEditorContainer );

	
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

	//Making the parser
	ScriptParser * parser = ScriptParser::makeRegular();

	QPushButton * compButt = new QPushButton( "Compile Selected" );
	connect( compButt, &QPushButton::clicked, [this,parser,march](){
		console->log( "Parsing started" );
		Block * script = parser->parse( this->codeEditTabs->currentWidget()->findChild<QTextEdit*>()->toPlainText().toStdString() );
		console->log( "Parsing ended" );
		console->log( "Compiling started" );

		std::string outlets = "";

		try
		{
			for( int i = 0; i < this->outlets->rowCount(); i++ )
			{
				outlets += ( "frac " + this->outlets->getName( i ) + " = " + std::to_string( this->outlets->getValue( i ) ).c_str() + ";" ).toStdString();
				script->variablesInScope[ this->outlets->getName( i ).toStdString() ] = VariablePacket( Value() );
			}

			if( ( this->codeEditTabs->currentIndex() ) == 0 )
			{
				script->variablesInScope[ "x" ] = VariablePacket( Value() );
				script->variablesInScope[ "y" ] = VariablePacket( Value() );
				script->variablesInScope[ "z" ] = VariablePacket( Value() );
				script->variablesInScope[ "r" ] = VariablePacket( Value(), true );
				script->variablesInScope[ "g" ] = VariablePacket( Value(), true );
				script->variablesInScope[ "b" ] = VariablePacket( Value(), true );

				std::ifstream if_a("Assets/clparts/clheader.cl", std::ios_base::binary);
				std::ifstream if_b("Assets/clparts/clfootergutsSUBSET.cl", std::ios_base::binary);
				std::ofstream of_c("Assets/temp/testAll.cl", std::ios_base::binary);

				std::string output = parser->textOf( script, 0, true );

				of_c << if_a.rdbuf() << "bool subsetFunction( frac x, frac y, frac z, frac * r, frac * g, frac * b ){ " << outlets << output << "}" << if_b.rdbuf();
				of_c.close();

				march->load( "Assets/temp/testAll.cl" );
			}
			else
			if( ( this->codeEditTabs->currentIndex() ) == 1 )
			{
				script->variablesInScope[ "x" ] = VariablePacket( Value() );
				script->variablesInScope[ "y" ] = VariablePacket( Value() );
				script->variablesInScope[ "z" ] = VariablePacket( Value() );
				script->variablesInScope[ "r" ] = VariablePacket( Value(), true );
				script->variablesInScope[ "g" ] = VariablePacket( Value(), true );
				script->variablesInScope[ "b" ] = VariablePacket( Value(), true );

				std::ifstream if_a("Assets/clparts/clheader.cl", std::ios_base::binary);
				std::ifstream if_b("Assets/clparts/clfooterguts.cl", std::ios_base::binary);
				std::ofstream of_c("Assets/temp/testAll.cl", std::ios_base::binary);

				std::string output = parser->textOf( script, 0, true );

				of_c << if_a.rdbuf() << "frac distanceFunction( frac x, frac y, frac z, frac * r, frac * g, frac * b, __global const float* sceneVarData ){ " << outlets << output << "}" << if_b.rdbuf();
				of_c.close();

				march->load( "Assets/temp/testAll.cl" );
			}
			else
			if( ( this->codeEditTabs->currentIndex() ) == 2 )
			{
				script->variablesInScope[ "x" ] = VariablePacket( Value() );
				script->variablesInScope[ "y" ] = VariablePacket( Value() );
				script->variablesInScope[ "z" ] = VariablePacket( Value() );
				script->variablesInScope[ "dx" ] = VariablePacket( Value() );
				script->variablesInScope[ "dy" ] = VariablePacket( Value() );
				script->variablesInScope[ "dz" ] = VariablePacket( Value() );
				script->variablesInScope[ "r" ] = VariablePacket( Value(), true );
				script->variablesInScope[ "g" ] = VariablePacket( Value(), true );
				script->variablesInScope[ "b" ] = VariablePacket( Value(), true );
				script->variablesInScope[ "dist" ] = VariablePacket( Value(), true );

				std::ifstream if_a("Assets/clparts/clheader.cl", std::ios_base::binary);
				std::ifstream if_b("Assets/clparts/clfootergutsTRACE.cl", std::ios_base::binary);
				std::ofstream of_c("Assets/temp/testAll.cl", std::ios_base::binary);

				std::string output = parser->textOf( script, 0, true );

				of_c << if_a.rdbuf() << "bool traceFunction( frac x, frac y, frac z, frac dx, frac dy, frac dz, frac * r, frac * g, frac * b, frac * dist ){ " << outlets << output << "}" << if_b.rdbuf();
				of_c.close();

				march->load( "Assets/temp/testAll.cl" );
			}
		}
		catch( char * error )
		{
			console->log( ( "UNEXPECTED COMPILATION ERROR: " + std::string( error ) ).c_str(), ConsoleWidget::MESSAGE_TYPE::MSG_DIREERROR );
		}
		console->log( "Compiling ended" );
	} );
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

	currentObjectContainer->addWidget( horizontalBottomBar );

	
	this->layout()->addWidget( currentObjectContainer );//Script edit widget
	currentObjectContainer->setVisible( false );

	outletContainer = new QWidget;
	outletContainer->setLayout( new QVBoxLayout );

	outletContainer->layout()->addWidget( new QLabel( "Outlets" ) );

	outlets = new ValueTableWidget();

	outletContainer->setFixedWidth( 150 );

	outletContainer->layout()->addWidget( outlets );

	QPushButton * addButt = new QPushButton( "Add" );
	connect( addButt, &QPushButton::clicked, [this](){
		outlets->addFullRow();
		hasChanged = true;
	} );
	outletContainer->layout()->addWidget( addButt );

	QPushButton * deleteButt = new QPushButton( "Delete" );
	connect( deleteButt, &QPushButton::clicked, [this](){
		if( outlets->currentRow() != -1 )
		{
			QMessageBox msgBox;
			msgBox.setWindowTitle("Deleting outlet");
			msgBox.setText("Are you sure you want to delete the outlet \"" + outlets->getName( outlets->currentRow() ) + "\"?");
			msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
			msgBox.setDefaultButton(QMessageBox::No);
			int result = msgBox.exec();
			if( result == QMessageBox::Yes )
			{
				outlets->removeRow( outlets->currentRow() );
				hasChanged = true;
			}
		}
	} );
	outletContainer->layout()->addWidget( deleteButt );

	this->layout()->addWidget( outletContainer ); //Outlet widget
	hasChanged = false;
	editingIndex = -1;
	hide();
}



void ObjectEditorWidget::changingSelection()
{
	if( objectList->currentRow() != -1 )
	{
		std::cout << "Changing Selection: " << objectList->currentRow() << " -- " << objectList->currentItem()->text().toStdString() << std::endl;
	
		if( editingIndex != objectList->currentRow() )
		{
			bool followThrough = true;
			if( hasChanged )
			{
				QMessageBox msgBox;
				msgBox.setWindowTitle("Unsaved Changes");
				msgBox.setText("Would you like to save the current object?");
				msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);
				msgBox.setDefaultButton(QMessageBox::Yes);
				int result = msgBox.exec();
				if( result == QMessageBox::Yes )
				{
					saveCurrent();
				}
				if( result == QMessageBox::Cancel )
				{
					followThrough = false;
				}
			}
			if( followThrough )
			{
				editingIndex = objectList->currentRow();
				display( StructureObject::load( objectList->currentItem()->text().toStdString() ) );
				hasChanged = false;
			}
			else
			{
				objectList->setCurrentRow( editingIndex );
			}
		}
	}

	if( editingIndex == -1 )
	{
		hide();
	}
}

void ObjectEditorWidget::keyPressEvent( QKeyEvent *e )
{
	if ( (e->key() == Qt::Key_S)  && QApplication::keyboardModifiers() && Qt::ControlModifier)
    {
		saveCurrent();
	}
}

void ObjectEditorWidget::saveCurrent()
{
	if( !noObjectSelected->isVisible() )
	{
		bool needsRefresh = false;
		QString nam = nameEditor->text();
		std::string nameOfObject = nam.toStdString();
		if( nam.compare( originalName ) != 0 )
		{
			remove( ( "Assets/objs/" + originalName + ".rpobj" ).toStdString().c_str() );
			needsRefresh = true;
		}

		std::ofstream of_c(	"Assets/objs/" + nameOfObject + ".rpobj", std::ios_base::binary);
		std::cout << "Saving" << std::endl;
	
		//of_c << nameOfObject;
		int count = outlets->rowCount();
		of_c.write( reinterpret_cast<char*>(&count), sizeof(count) );
		for( int i = 0; i < outlets->rowCount(); i++ )
		{
			std::string outName = outlets->getName( i ).toStdString();
			int outNameLength = outName.length();
			of_c.write( reinterpret_cast<char*>(&outNameLength), sizeof(outNameLength) );
			of_c << outName;

			float num = outlets->getValue( i );
			of_c.write( reinterpret_cast<char*>( &num ), sizeof(num) );
		}

		for( int i = 0; i < codeEditTabs->count(); i++ )
		{
			bool enabled = codeEditTabs->widget( i )->findChild<QCheckBox*>()->isChecked();
			of_c.write( reinterpret_cast<char*>(&enabled), sizeof(enabled) );

			std::string code = codeEditTabs->widget( i )->findChild<QTextEdit*>()->toPlainText().toStdString();
			int codeLength = code.length();

			of_c.write( reinterpret_cast<char*>(&codeLength), sizeof(codeLength) );
			of_c << code;
		}

		of_c.close();
	
		originalName = nam;
		hasChanged = false;
		if( needsRefresh )
		{
			objectList->refreshList();
			objectList->seekObject( nam );
		}

	}
}

void ObjectEditorWidget::display( StructureObject obj )
{
	noObjectSelected->setVisible( false );
	currentObjectContainer->setVisible( true );
	outletContainer->setVisible( true );

	originalName = obj.name.c_str();

	nameEditor->setText( obj.name.c_str() );
	
	outlets->eraseAll();

	for( int i = 0; i < obj.numOutlets; i++ )
	{
		outlets->addFullRow();

		outlets->setName( i, obj.outletNames[i].c_str() );
		outlets->setValue( i, obj.outletValues[i] );
	}

	for( int i = 0; i < codeEditTabs->count(); i++ )
	{
		codeEditTabs->widget( i )->findChild<QCheckBox*>()->setChecked( obj.enabledModes[i] );

		codeEditTabs->widget( i )->findChild<QTextEdit*>()->setText( obj.codeBlocks[i].c_str() );
	}
}

void ObjectEditorWidget::hide()
{
	noObjectSelected->setVisible( true );
	currentObjectContainer->setVisible( false );
	outletContainer->setVisible( false );
}