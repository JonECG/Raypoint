#include "EmptyNode.h"
#include "Value.h"

Value EmptyNode::evaluate()
{
	return Value();
}

bool EmptyNode::checkSyntaxSpecial()
{
	return children.size() == 0;
}