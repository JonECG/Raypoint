#include "ValueNode.h"

ValueNode::ValueNode( float floatVal )
{
	this->val = Value(floatVal);
}
Value ValueNode::evaluate()
{
	return val;
}
bool ValueNode::checkSyntaxSpecial()
{
	return children.size() == 0;
}