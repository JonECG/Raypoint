#include "MainWindow.h"
#include "QtWidgets\qlayout.h"
#include "QtWidgets\qtabwidget.h"
#include "ObjectEditorWidget.h"
#include "SceneEditorWidget.h"
#include "AnimationEditorWidget.h"
#include "RenderSettingsWidget.h"
#include "SceneModel.h"

MainWindow::MainWindow()
{
	tabs = new QTabWidget( this );
	scene = new SceneModel;
	SceneVariable cam( "Camera" );
	cam.addValue( "From X", 10 );
	cam.addValue( "From Y", 0 );
	cam.addValue( "From Z", 0 );
	cam.addValue( "To X", 0 );
	cam.addValue( "To Y", 0 );
	cam.addValue( "To Z", 0 );
	cam.addValue( "Up X", 0 );
	cam.addValue( "Up Y", 1 );
	cam.addValue( "Up Z", 0 );
	cam.addValue( "FOV", 50 );
	scene->variables.push_back( cam );

	QWidget * test = new QWidget;

	tabs->addTab( new ObjectEditorWidget(), tr("Object Editor"));
	tabs->addTab( new SceneEditorWidget( &scene ), tr("Scene Editor"));
	tabs->addTab( new AnimationEditorWidget( &scene ), tr("Animation Editor"));
	tabs->addTab( new RenderSettingsWidget( &scene ), tr("Render Settings"));

	this->setLayout( new QVBoxLayout() );

	this->layout()->addWidget( tabs );
}
