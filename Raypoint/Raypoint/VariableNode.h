#pragma once
#include "ScriptNode.h"
#include <string>
class Value;

class VariableNode : public ScriptNode
{
	std::string identifier;
public:
	VariableNode( std::string ident );
	Value evaluate();
	std::string getIdentifier();
	bool checkSyntaxSpecial();
};

