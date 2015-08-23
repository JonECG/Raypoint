#include "SceneModel.h"
#include "CatmullRom.h"
#include "NewRayMarchWidget.h"

#include "ScriptParser.h"
#include "Block.h"
#include <fstream>

#include <QtWidgets\qlineedit.h>
#include "Block.h"
#include "Camera.h"
#include <QtWidgets\qcheckbox.h>
#include "InSceneJointWidget.h"
#include "InSceneStructureWidget.h"

float SceneVariable::calcValue( int valueIndex, float time )
{
	float result = 0;

	if( animation.size() == 0 )
	{
		//DEFAULT
		result = defaultValues[valueIndex];
	}
	else
	if( animation.size() == 1 )
	{
		//ALL THE SAME
		result = animation[0].values[valueIndex];
	}
	else
	if( animation.size() == 2 )
	{
		//LINEAR INTERPOLATION
		result = animation[0].values[valueIndex] + (animation[1].values[valueIndex] - animation[0].values[valueIndex]) * ( (time-animation[0].time)/(animation[1].time-animation[0].time) );
	}
	else
	{
		//CENTRIPETAL CATMULL-ROM
		float datums[4];
		float times[4];

		int foundLesser;
		bool found = false;
		for( unsigned int i = 0; !found && i < animation.size(); i++ )
		{
			foundLesser = i;
			if( animation[i].time > time )
			{
				found = true;
				foundLesser--;
			}
		}

		for( int i = 0; i < 4; i++ )
		{
			int actIndex = foundLesser + i - 1;
			int normalizedActIndex = std::max( 0, std::min( (int)animation.size()-1, actIndex ) );
			datums[i] = animation[normalizedActIndex].values[valueIndex];
			times[i] = animation[normalizedActIndex].time;
			if( actIndex != normalizedActIndex )
			{
				int ref = normalizedActIndex - ( actIndex - normalizedActIndex );
				times[i] = times[i] + ( times[i]-animation[ref].time );
			}
		}

		float interpPower = 1.0f;

		//Time-based interpolation
		float relTimes[4];
		float relTime;
		relTimes[0] = 0;
		for( int i = 1; i < 4; i++ )
		{
			relTimes[i] = std::pow( std::abs( times[i] - times[i-1] ), interpPower ) + relTimes[i-1];
		}
		relTime = relTimes[1] + (relTimes[2] - relTimes[1]) * ( (time - times[1])/(times[2]-times[1]));
		result = CatmullRom::interpolate<float>( datums, relTimes, relTime );
		//result = CatmullRom::interpolate<float>( datums, times, time );

		//Distance-based interpolation -- Broken in concept : I was an idiot for trying
		/*
		float dists[4];
		float dist;
		dists[0] = 0;
		for( int i = 1; i < 4; i++ )
		{
			dists[i] = std::pow( std::abs( datums[i] - datums[i-1] ), interpPower ) + dists[i-1];
		}
		dist = dists[1] + (dists[2] - dists[1]) * ( (time - times[1])/(times[2]-times[1]));
		result = CatmullRom::interpolate<float>( datums, dists, dist );
		*/
	}

	return result;
}

void stringReplaceAll( std::string * source, std::string search, std::string replace )
{
	std::string::size_type n = 0;
	while ( ( n = source->find( search, n ) ) != std::string::npos )
	{
		source->replace( n, search.size(), replace );
		n += replace.size();
	}
}

std::string SceneModel::getStructureRepresentation( ScriptParser * parser, InSceneStructureWidget * structure, int index )
{
	//Write accessors to the scene variables
	std::string sceneVars = "";
	for( unsigned int i = 1; i < variables.size(); i++ )
	{
		sceneVars += "\r\tfrac " + variables[i].name + " = sceneVarData["+std::to_string(i)+"];";
	}
	sceneVars += "\r\r";

	//Parse the scripts that represent the outlets in relation to scene variables
	auto chil = structure->outletList->findChildren<QLineEdit*>();
	std::string outletStatements = "";
	for( int i = 0; i < chil.size(); i++ )
	{
		std::string s = chil.at(i)->text().toStdString();
		ScriptNode * node = parser->parseLine( s );
		Block b;
		b.addChild( node );
		for( unsigned int j = 1; j < variables.size(); j++ )
		{
			b.variablesInScope[ variables[j].name ] = VariablePacket( Value() );
		}
		outletStatements += "\r\tfrac " + structure->st.outletNames[ i ] + " = " + ScriptParser::textOf( node, 0, false ) + ";";
	}
	outletStatements += "\r\r";

	StructureObject structObject = StructureObject::load( structure->st.name );
	Block * script = parser->parse( structObject.codeBlocks[ structure->currentType ] );

	//Declare outlets
	for( int i = 0; i < structure->st.numOutlets; i++ )
	{
		script->variablesInScope[ structure->st.outletNames[i] ] = VariablePacket( Value() );
	}

	//Declare parameters
	script->variablesInScope[ "x" ] = VariablePacket( Value() );
	script->variablesInScope[ "y" ] = VariablePacket( Value() );
	script->variablesInScope[ "z" ] = VariablePacket( Value() );
	script->variablesInScope[ "r" ] = VariablePacket( Value(), true );
	script->variablesInScope[ "g" ] = VariablePacket( Value(), true );
	script->variablesInScope[ "b" ] = VariablePacket( Value(), true );

	//Pull ray cast functionality part
	std::string rayPart = "";
	std::string scriptPart = "";
	if( structure->currentType == 0 )
	{
		std::ifstream t("Assets/clparts/clpartSUBSET.cl");
		rayPart = std::string((std::istreambuf_iterator<char>(t)),
							std::istreambuf_iterator<char>());
		t.close();
	}
	else
	if( structure->currentType == 1 )
	{
		std::ifstream t("Assets/clparts/clpartDISTANCE.cl");
		rayPart = std::string((std::istreambuf_iterator<char>(t)),
							std::istreambuf_iterator<char>());
		t.close();
	}
	else
	if( structure->currentType == 2 )
	{
		script->variablesInScope[ "dx" ] = VariablePacket( Value() );
		script->variablesInScope[ "dy" ] = VariablePacket( Value() );
		script->variablesInScope[ "dz" ] = VariablePacket( Value() );
		script->variablesInScope[ "dist" ] = VariablePacket( Value(), true );

		std::ifstream t("Assets/clparts/clpartTRACE.cl");
		rayPart = std::string((std::istreambuf_iterator<char>(t)),
							std::istreambuf_iterator<char>());
		t.close();
	}

	//Seam everything together and replace hotlinks
	scriptPart = sceneVars + outletStatements + parser->textOf( script, 0, true );
	stringReplaceAll( &rayPart, "$$FUNCTIONINJECTION$$", scriptPart );
	stringReplaceAll( &rayPart, "$$NUM$$", std::to_string( index ) );
	
	return rayPart + "\n\n";
}

int SceneModel::traverseJointRelationships( InSceneJointWidget * joint, std::string * partsString, std::string * sceneCalcString, ScriptParser * parser, int index, int tabs )
{
	std::string tabString = "";
	for( int i = 0; i < tabs; i++ )
	{
		tabString += '\t';
	}

	int relIndex = index;
	if( joint->isSingle )
	{
		*partsString += getStructureRepresentation( parser, joint->structure, relIndex );
		*sceneCalcString += tabString + "marchPacket( distanceFunction_" + std::to_string( relIndex ) + "( origin, direction, &heldColor, &heldHit, sceneVars ), heldHit ? " + std::to_string( relIndex ) + " : -1, heldColor )";
		relIndex++;
	}
	else
	{
		*sceneCalcString += tabString + "unionOp" + "(\n";
		relIndex = traverseJointRelationships( joint->first, partsString, sceneCalcString, parser, relIndex, tabs + 1 );
		*sceneCalcString += ",\n";
		relIndex = traverseJointRelationships( joint->second, partsString, sceneCalcString, parser, relIndex, tabs + 1 );
		*sceneCalcString += "\n" + tabString + ")";
	}

	return relIndex;
}

void SceneModel::visualize( NewRayMarchWidget * widg, bool propogateErrors )
{
	std::string file;
	ScriptParser * parser = ScriptParser::makeRegular();

	InSceneJointWidget * topJoint = new InSceneJointWidget( structures[0] );
	for( unsigned int structureIndex = 1; structureIndex < structures.size(); structureIndex++ )
	{
		topJoint = new InSceneJointWidget( topJoint, new InSceneJointWidget( structures[structureIndex] ) );
	}

	try
	{
		std::ifstream t("Assets/clparts/clheader.cl");
		std::string header((std::istreambuf_iterator<char>(t)),
						 std::istreambuf_iterator<char>());
		t.close();

		file += header;


		std::string partsString = "";
		std::string sceneCalcString = "";

		//Get the string representing all of the parts as well as their tree structure
		traverseJointRelationships( topJoint, &partsString, &sceneCalcString, parser );
		file += partsString;

		//Create all of the cases for the normal switch statement
		std::string normalCases = "";
		for( unsigned int structureIndex = 0; structureIndex < structures.size(); structureIndex++ )
		{
			normalCases += "\t\tcase " + std::to_string( structureIndex ) + ": result = normalFunction_" + std::to_string( structureIndex ) + "( origin, direction, distTraveled, sceneVars ); break;\n";
		}

		//Draw everything together
		std::ifstream inj("Assets/clparts/clcalcinjection.cl");
		std::string calcInjection = std::string((std::istreambuf_iterator<char>(inj)),
						std::istreambuf_iterator<char>());
		inj.close();
		stringReplaceAll( &calcInjection, "$$DISTCALCINJECTION$$", sceneCalcString );
		stringReplaceAll( &calcInjection, "$$NORMALINJECTION$$", normalCases );
		file += calcInjection;


		//Put the core kernel in
		std::ifstream ft("Assets/clparts/clcombinedfooter.cl");
		std::string footer = std::string((std::istreambuf_iterator<char>(ft)),
						std::istreambuf_iterator<char>());
		ft.close();
		file += footer;

		std::cout << file << std::endl;


		std::ofstream of_c("Assets/temp/testAll.cl", std::ios_base::binary);
		of_c << file;
		of_c.close();
		
		widg->load( "Assets/temp/testAll.cl" );
	}
	catch( char * except )
	{
		if( propogateErrors )
			throw except;
	}
}

Camera SceneModel::calcCamera( float time )
{
	Camera cam;
	cam.from = glm::dvec3( variables[0].calcValue( 0, time ), variables[0].calcValue( 1, time ), variables[0].calcValue( 2, time ) );
	cam.to	 = glm::dvec3( variables[0].calcValue( 3, time ), variables[0].calcValue( 4, time ), variables[0].calcValue( 5, time ) );
	cam.up	 = glm::dvec3( variables[0].calcValue( 6, time ), variables[0].calcValue( 7, time ), variables[0].calcValue( 8, time ) );
	cam.fov	 = variables[0].calcValue( 9, time );
	
	return cam;
}

SceneModel::SceneModel()
{
	autoSave = false;
}

void SceneModel::save( std::string path )
{
	std::ofstream of_c( path, std::ios_base::binary);
	
	int count = structures.size();
	of_c.write( reinterpret_cast<char*>(&count), sizeof(count) );
	for( int i = 0; i < count; i++ )
	{
		std::string structName = structures[i]->st.name;
		int structNameLength = structName.length();
		of_c.write( reinterpret_cast<char*>(&structNameLength), sizeof(structNameLength) );
		of_c << structName;

		int outletCount = structures[i]->st.numOutlets;
		of_c.write( reinterpret_cast<char*>(&outletCount), sizeof(outletCount) );

		//Outlets
		auto useDefaults = structures[i]->outletList->findChildren<QCheckBox*>();
		auto lineEdits = structures[i]->outletList->findChildren<QLineEdit*>();
		for( int j = 0; j < outletCount; j++ )
		{
			bool useDefault = useDefaults.at( j )->isChecked();
			of_c.write( reinterpret_cast<char*>(&useDefault), sizeof(useDefault) );

			if( !useDefault )
			{
				std::string subscript = lineEdits.at( j )->text().toStdString();
				int subscriptLength = subscript.length();

				of_c.write( reinterpret_cast<char*>(&subscriptLength), sizeof(subscriptLength) );
				of_c << subscript;
			}
		}
	}

	int variableCount = variables.size();
	of_c.write( reinterpret_cast<char*>(&variableCount), sizeof(variableCount) );
	for( int i = 0; i < variableCount; i++ )
	{
		int variableNameLength = variables[i].name.length();
		of_c.write( reinterpret_cast<char*>(&variableNameLength), sizeof(variableNameLength) );
		of_c << variables[i].name;

		int variablePartCount = variables[i].numUsed;
		of_c.write( reinterpret_cast<char*>(&variablePartCount), sizeof(variablePartCount) );
		for( int j = 0; j < variablePartCount; j++ )
		{
			int variablePartNameLength = std::string( variables[i].valueNames[j] ).length();
			of_c.write( reinterpret_cast<char*>(&variablePartNameLength), sizeof(variablePartNameLength) );
			of_c << variables[i].valueNames[j];

			float variablePartDefault = variables[i].defaultValues[j];
			of_c.write( reinterpret_cast<char*>(&variablePartDefault), sizeof(variablePartDefault) );
		}
	}

	for( int i = 0; i < variableCount; i++ )
	{
		int numKeyPoints = variables[i].animation.size();
		of_c.write( reinterpret_cast<char*>(&numKeyPoints), sizeof(numKeyPoints) );

		for( int j = 0; j < numKeyPoints; j++ )
		{
			float time = variables[i].animation[j].time;
			of_c.write( reinterpret_cast<char*>(&time), sizeof(time) );

			for( int k = 0; k < variables[i].numUsed; k++ )
			{
				float val = variables[i].animation[j].values[k];
				of_c.write( reinterpret_cast<char*>(&val), sizeof(val) );
			}
		}
	}


	of_c.close();

	this->path = path;
	this->autoSave = true;
}

SceneModel SceneModel::load( std::string path )
{
	SceneModel result;

	result.path = path;
	result.autoSave = true;

	std::ifstream if_c(	path, std::ios_base::binary);

	
	int count = 0;
	if_c.read( reinterpret_cast<char*>(&count), sizeof(count) );
	for( int i = 0; i < count; i++ )
	{
		int structNameLength = 0;
		if_c.read( reinterpret_cast<char*>(&structNameLength), sizeof(structNameLength) );

		char * name = new char[ structNameLength + 1 ];
		if_c.read( name, structNameLength );
		name[ structNameLength ] = '\0';
		std::string structName( name );

		StructureObject st = StructureObject::load( structName );

		InSceneStructureWidget * widg = new InSceneStructureWidget( st );

		delete name;

		int outletCount = 0;
		if_c.read( reinterpret_cast<char*>(&outletCount), sizeof(outletCount) );

		//Outlets
		auto useDefaults = widg->outletList->findChildren<QCheckBox*>();
		auto lineEdits = widg->outletList->findChildren<QLineEdit*>();
		for( int j = 0; j < outletCount && j < widg->st.numOutlets; j++ )
		{
			bool useDefault = false;
			if_c.read( reinterpret_cast<char*>(&useDefault), sizeof(useDefault) );

			useDefaults.at( j )->setChecked( useDefault );

			if( !useDefault )
			{
				int subscriptLength = 0;
				if_c.read( reinterpret_cast<char*>(&subscriptLength), sizeof(subscriptLength) );

				char * subscript = new char[ subscriptLength + 1 ];
				if_c.read( subscript, subscriptLength );
				subscript[ subscriptLength ] = '\0';

				lineEdits.at( j )->setText( QWidget::tr( subscript ) );

				delete subscript;
			}
		}

		result.structures.push_back( widg );
	}


	int variableCount = 0;
	if_c.read( reinterpret_cast<char*>(&variableCount), sizeof(variableCount) );
	for( int i = 0; i < variableCount; i++ )
	{
		int variableNameLength = 0;
		if_c.read( reinterpret_cast<char*>(&variableNameLength), sizeof(variableNameLength) );

		char * varName = new char[ variableNameLength + 1 ];
		if_c.read( varName, variableNameLength );
		varName[ variableNameLength ] = '\0';

		result.variables.push_back( SceneVariable( std::string( varName ) ) );
		SceneVariable * currentVar = &result.variables[i];

		delete varName;


		int variablePartCount = 0;
		if_c.read( reinterpret_cast<char*>(&variablePartCount), sizeof(variablePartCount) );
		for( int j = 0; j < variablePartCount; j++ )
		{
			int variablePartNameLength = 0;
			if_c.read( reinterpret_cast<char*>(&variablePartNameLength), sizeof(variablePartNameLength) );

			char * varPartName = new char[ variablePartNameLength + 1 ];
			if_c.read( varPartName, variablePartNameLength );
			varPartName[ variablePartNameLength ] = '\0';

			float varPartDefault = 0;
			if_c.read( reinterpret_cast<char*>(&varPartDefault), sizeof(varPartDefault) );

			currentVar->addValue( varPartName, varPartDefault );
		}
	}

	for( int i = 0; i < variableCount; i++ )
	{
		int numKeyPoints = 0;
		if_c.read( reinterpret_cast<char*>(&numKeyPoints), sizeof(numKeyPoints) );

		
		for( int j = 0; j < numKeyPoints; j++ )
		{
			float time = 0;
			if_c.read( reinterpret_cast<char*>(&time), sizeof(time) );

			KeyPoint point = result.variables[i].makePoint( time );

			for( int k = 0; k < result.variables[i].numUsed; k++ )
			{
				float val = 0;
				if_c.read( reinterpret_cast<char*>(&val), sizeof(val) );

				point.values[k] = val;
			}

			result.variables[i].animation.push_back( point );
		}
	}

	if_c.close();
	
	return result;
}