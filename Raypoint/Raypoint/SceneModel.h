#pragma once
#include <vector>
class NewRayMarchWidget;
class InSceneJointWidget;
class InSceneStructureWidget;
class Camera;
class ScriptParser;

struct KeyPoint 
{
	float time;
	int numUsed;
	float values[10];
	
	inline KeyPoint( float time = 0, int numUsed = 0 ){ this->time = time; this->numUsed = numUsed; }
};

struct SceneVariable 
{
	std::string name;
	float defaultValues[10];

	int numUsed;
	const char * valueNames[10];

	std::vector< KeyPoint > animation;

	inline SceneVariable( std::string name = "" ){ this->name = name; numUsed = 0; }
	inline KeyPoint makePoint( float time = 0 ){ 
		KeyPoint point( time, numUsed );
		for( int i = 0; i < numUsed; i++ )
		{
			point.values[i] = defaultValues[i];
		}
		return point;
	}
	inline void addValue( const char * name, float defaultValue = 0 ){ valueNames[numUsed] = name; defaultValues[numUsed] = defaultValue; numUsed++; }

	float calcValue( int valueIndex, float time );
};

struct SceneModel
{
	std::string path;
	bool autoSave;

	std::vector< InSceneStructureWidget * > structures;
	std::vector< SceneVariable > variables;

	SceneModel();

	void visualize( NewRayMarchWidget * widg, bool propogateErrors = false );
	Camera calcCamera( float time = 0 );

	void save( std::string path );
	static SceneModel load( std::string path );
private:
	std::string getStructureRepresentation( ScriptParser * parser, InSceneStructureWidget * structure, int index );
	int traverseJointRelationships( InSceneJointWidget * joint, std::string * partsString, std::string * sceneCalcString, ScriptParser * parser, int index = 0, int tabs = 2 );
};

