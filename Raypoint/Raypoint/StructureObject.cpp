#include "StructureObject.h"
#include <fstream>

StructureObject StructureObject::load( std::string name )
{
	StructureObject result;

	result.name = name;

	std::ifstream if_c(	"Assets/objs/" + name + ".rpobj", std::ios_base::binary);

	if_c.read( reinterpret_cast<char*>(&result.numOutlets), sizeof( result.numOutlets ) ); 

	for( int i = 0; i < result.numOutlets; i++ )
	{
		int outNameLength;
		if_c.read( reinterpret_cast<char*>(&outNameLength), sizeof(outNameLength ) ); 


		char * name = new char[ outNameLength + 1 ];
		if_c.read( name, outNameLength );
		name[ outNameLength ] = '\0';
		std::string outName( name );

		float outValue;
		if_c.read( reinterpret_cast<char*>(&outValue), sizeof(outValue ) ); 

		result.outletNames[ i ] = outName;
		result.outletValues[ i ] = outValue;

		delete [] name;
	}

	for( int i = 0; i < 3; i++ )
	{
		if_c.read( reinterpret_cast<char*>(&result.enabledModes[i]), sizeof( result.enabledModes[i] ) ); 

		int codeLength;
		if_c.read( reinterpret_cast<char*>(&codeLength), sizeof(codeLength ) ); 

		char * code = new char[ codeLength + 1 ];
		if_c.read( code, codeLength );
		code[ codeLength ] = '\0';
		std::string codeText( code );

		result.codeBlocks[ i ] = codeText;

		delete [] code;
	}

	if_c.close();
	
	return result;
}