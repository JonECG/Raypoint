#pragma once
#include "ScriptNode.h"
class EmptyNode : public ScriptNode
{
public:
	Value evaluate();
	bool checkSyntaxSpecial();
};

