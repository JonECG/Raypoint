#pragma once
#include "ScriptNode.h"
class Value;
class Operator;

class OperatorNode : public ScriptNode
{
	Operator * op;

public:
	OperatorNode( Operator * op );
	int getPriority();
	Operator * getOperator();
	Value evaluate();
	bool checkSyntaxSpecial();
};

