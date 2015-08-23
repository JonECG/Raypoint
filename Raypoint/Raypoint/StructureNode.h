#pragma once
#include "ScriptNode.h"
#include "Structure.h"
class Value;
class Block;
class StructureNode : public ScriptNode
{
	Structure* structure;
	Block * inner;
public:
	StructureNode( Structure* structure );
	Block * findClosestScope();
	Block * getInner();
	Structure* getStructure();
	void setInner( Block * inner );
	Value evaluate();
	bool checkSyntaxSpecial();
};

