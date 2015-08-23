#include "Block.h"
#include "Value.h"

Block::Block()
{
	variablesInScope = std::map<std::string,VariablePacket>();
	//expressions = new ArrayList<ScriptNode>();
}

Block* Block::findClosestScope()
{
	return this;
}

Value Block::evaluate()
{
	for( unsigned int i = 0; i < children.size(); i++ )
	{
		children[ i ]->evaluate();
	}
	return Value(); //Nothing
}

bool Block::checkSyntaxSpecial()
{
	return children.size() > 0;
}