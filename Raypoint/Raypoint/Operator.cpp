#include "Operator.h"
#include "Value.h"

Operator::Operator(std::string symbol, int priority)
{
	this->symbol = symbol;
	this->priority = priority;
}
std::string Operator::getSymbol()
{
	return symbol;
}
int Operator::getPriority()
{
	return priority;
}