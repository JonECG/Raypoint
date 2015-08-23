#pragma once
#include "ScriptNode.h"
class Value;

class OperatorNode : public ScriptNode
{
public:
	OperatorNode();
	int getPriority();
	Value evaluate();
	bool CheckSyntaxSpecial();
};

