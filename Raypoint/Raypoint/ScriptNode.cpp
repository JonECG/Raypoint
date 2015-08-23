#include "ScriptNode.h"
#include "Block.h"

ScriptNode::ScriptNode()
{
	children = std::vector<ScriptNode*>();
	parent = nullptr;
}

std::vector<ScriptNode*> * ScriptNode::getChildren()
{
	return &children;
}
	
int ScriptNode::getPriority()
{
	return 100000;
}

void ScriptNode::addChild( ScriptNode * child )
{
	child->parent = this;
	children.push_back( child );
}
	
Block* ScriptNode::findClosestScope()
{
	return (parent == nullptr) ? nullptr : parent->findClosestScope();
}

bool ScriptNode::checkSyntax()
{
	bool result = checkSyntaxSpecial();
	for( unsigned int i = 0; i < children.size() && result; i++ )
	{
		result = children[ i ]->checkSyntax();
	}
	return result;
}