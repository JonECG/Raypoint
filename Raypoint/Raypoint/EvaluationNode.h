#pragma once
class Value;
#include "ScriptNode.h"

class EvaluationNode : public ScriptNode
{
public:
	Value evaluate();
	bool checkSyntaxSpecial();
};

