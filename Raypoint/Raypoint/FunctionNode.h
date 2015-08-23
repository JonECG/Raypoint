#pragma once
#include "ScriptNode.h"
class Value;
class Function;

class FunctionNode : public ScriptNode
{
	Function * function;
public:
	FunctionNode( Function * function );
	Function * getFunction();
	Value evaluate();
	bool checkSyntaxSpecial();
};