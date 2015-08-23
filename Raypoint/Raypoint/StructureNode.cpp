#include "StructureNode.h"
#include "Structure.h"
#include "Value.h"
#include "Block.h"

StructureNode::StructureNode( Structure* structure )
{
	this->structure = structure;
}
Block * StructureNode::findClosestScope()
{
	inner->parent = parent;
	return inner;
}
Block * StructureNode::getInner()
{
	inner->parent = parent;
	return inner;
}
Structure* StructureNode::getStructure()
{
	return structure;
}
void StructureNode::setInner( Block * inner )
{
	this->inner = inner;
	inner->parent = parent;
}
Value StructureNode::evaluate()
{
	return structure->evaluate( this );
}
bool StructureNode::checkSyntaxSpecial()
{
	return structure->checkSyntax( this );
}