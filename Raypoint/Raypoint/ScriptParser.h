#pragma once
#include <map>
#include "Structure.h"
#include <functional>
class Function;
class Operator;
class Block;
class ScriptNode;

class ScriptParser
{
	std::map< std::string, Function* > functions;
	std::map< std::string, Structure* > structures;
	std::map< std::string, Operator* > operators;

public:
	ScriptParser();
	
	void registerOperator( Operator * op );
	void registerStructure( Structure* structure );
	void registerFunction( Function * funct );

	Block * parse( std::string function );
	Block * parseBlock( std::string block, int begin, int end );

	ScriptNode * parseLine( std::string line );
	ScriptNode * parseLine( std::string line, int begin, int end, ScriptNode* procedingExpression );
	ScriptNode * parseTokensRec( ScriptNode ** tokens, int begin, int end );

	static ScriptParser * makeRegular();
	static std::string textOf( ScriptNode * node, int indents, bool curly );
};

