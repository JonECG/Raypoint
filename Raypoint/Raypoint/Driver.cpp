#ifndef _DEBUG
	#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif

#include <iostream>

#include <QtWidgets\qapplication.h>
#include "MainWindow.h"

int main( int argc, char * argv[] )
{
	QApplication app( argc, argv );

	MainWindow* wind = new MainWindow();
	wind->show();
	return app.exec();
}