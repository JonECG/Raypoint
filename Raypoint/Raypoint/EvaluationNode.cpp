#include "EvaluationNode.h"

#include "Value.h"

Value EvaluationNode::evaluate()
{
	return children[ 0 ]->evaluate();
}

bool EvaluationNode::checkSyntaxSpecial()
{
	return children.size() == 1;
}