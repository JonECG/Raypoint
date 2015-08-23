#include <iostream>

#include <Qt\qapplication.h>
#include "RayCastHandle.h"



int main( int argc, char * argv[] )
{
	QApplication app( argc, argv );

	//

	RayCastHandle* pub = new RayCastHandle();
	//pub->resize( 128, 128 );
	//pub->show();
	pub->init();
	//return app.exec();
}