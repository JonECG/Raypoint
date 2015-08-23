#include "Function.h"


Function::Function(std::string identifier)
{
	this->identifier = identifier;
}


std::string Function::getIdentifier()
{
	return identifier;
}
