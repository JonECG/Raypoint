#include "VariableNode.h"
#include "Value.h"
#include "Block.h"

VariableNode::VariableNode( std::string ident )
{
	this->identifier = ident;
}
Value VariableNode::evaluate()
{
	bool found = false;
	Value result;
		
	Block * scope = findClosestScope();
		
	while( !found && scope != nullptr )
	{
		auto it = scope->variablesInScope.find( identifier );
		if( it != scope->variablesInScope.end() )
		{
			found = true;
			result = scope->variablesInScope.find( identifier )->second.getValue(); 
		}
		scope = ( scope->parent == nullptr ) ? nullptr : scope->parent->findClosestScope();
	}
		
	if( !found )
		throw "Variable not declared";
		
	return result;
}
bool VariableNode::checkSyntaxSpecial()
{
	return false;
}
std::string VariableNode::getIdentifier()
{
	return identifier;
}