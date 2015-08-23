#pragma once
#include "ScriptNode.h"
#include "Value.h"

class ValueNode : public ScriptNode
{
	Value val;
public:
	ValueNode( float floatVal );
	Value evaluate();
	bool checkSyntaxSpecial();
};