#pragma once
#include "ScriptNode.h"
#include <map>
#include "VariablePacket.h"

class Block : public ScriptNode
{
public:
	std::map<std::string,VariablePacket> variablesInScope;
	Block();
	Block* findClosestScope();
	Value evaluate();
	bool checkSyntaxSpecial();
};

