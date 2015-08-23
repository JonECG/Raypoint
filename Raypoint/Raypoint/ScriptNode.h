#pragma once
#include <vector>
class Block;
class Value;
class ScriptParser;
class ScriptNode
{
protected:
	std::vector<ScriptNode*> children;

public:
	ScriptNode * parent;
	ScriptNode();
	std::vector<ScriptNode*> * getChildren();
	virtual int getPriority();
	void addChild( ScriptNode * child );
	virtual Block* findClosestScope();
	virtual Value evaluate() = 0;
	virtual bool checkSyntaxSpecial() = 0;
	bool checkSyntax();
	friend ScriptParser;
};

