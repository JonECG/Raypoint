#include "OperatorNode.h"
#include "Operator.h"
#include "Value.h"

OperatorNode::OperatorNode( Operator * op )
{
	this->op = op;
}

int OperatorNode::getPriority()
{
	return op->getPriority();
}

Operator* OperatorNode::getOperator()
{
	return op;
}

Value OperatorNode::evaluate()
{
	return Value();
}

bool OperatorNode::checkSyntaxSpecial()
{
	return false;
}