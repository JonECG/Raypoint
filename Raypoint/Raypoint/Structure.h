#pragma once
#include <string>
#include "Value.h"
#include <functional>
class StructureNode;

class Structure
{
	std::string identifier;
	std::function<Value(StructureNode*)> evaluation;
	std::function<bool(StructureNode*)> syntaxCheck;

public:
	inline Structure(std::string identifier, std::function<Value(StructureNode*)> evaluation, std::function<bool(StructureNode*)> syntaxCheck )
	{
		this->identifier = identifier;
		this->evaluation = evaluation;
		this->syntaxCheck = syntaxCheck;
	}
	inline std::string getIdentifier()
	{
		return identifier;
	}
	inline Value evaluate( StructureNode * node )
	{
		return evaluation( node );
	}
	inline bool checkSyntax( StructureNode * node )
	{
		return syntaxCheck( node );
	}
};

