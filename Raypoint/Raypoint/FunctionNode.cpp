#include "FunctionNode.h"
#include "Function.h"
#include "Value.h"

FunctionNode::FunctionNode( Function * function )
{
	this->function = function;
}
Function * FunctionNode::getFunction()
{
	return this->function;
}
Value FunctionNode::evaluate()
{
	return Value();
}
bool FunctionNode::checkSyntaxSpecial()
{
	return false;
}