#include "ObjectListWidget.h"
#include <vector>
#include <Windows.h>
#include <iostream>

ObjectListWidget::ObjectListWidget(void)
{
}

std::vector< std::string > getAllObjectsInDir( std::string directory, std::string ext)
{
	std::vector<std::string> result;

	WIN32_FIND_DATA data;
	HANDLE h = FindFirstFile( ( directory + std::string( "*." ) + ext).c_str(),&data);
	
	if( h!=INVALID_HANDLE_VALUE ) 
	{
		do
		{
			char* nPtr = new char [lstrlen( data.cFileName ) + 1 - ( ext.length() + 1 )];
			for( unsigned int i = 0; i < lstrlen( data.cFileName ) - ( ext.length() + 1 ); i++ )
				nPtr[i] = char( data.cFileName[i] );

			nPtr[lstrlen( data.cFileName ) - ( ext.length() + 1 )] = '\0';
			result.push_back( nPtr );

		} while(FindNextFile(h,&data));
	} 
	else 
		std::cout << "Error: No such folder." << std::endl;
	
	FindClose(h);

	return result;
}

void ObjectListWidget::refreshList()
{
	clear();

	auto files = getAllObjectsInDir( "Assets/objs/", "rpobj" );
	for( unsigned int i = 0; i < files.size(); i++ )
	{
		addItem( files[i].c_str() );
	}

}
void ObjectListWidget::seekObject( QString string )
{
	bool found = false;
	for( int i = 0; i < count() && !found; i++ )
	{
		if( item( i )->text().compare( string ) == 0 )
		{
			found = true;
			setCurrentRow( i );
		}
	}
}