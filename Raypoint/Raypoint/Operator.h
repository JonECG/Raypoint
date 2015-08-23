#pragma once
#include <string>
class Value;

class Operator
{
	int priority;
	std::string symbol;
public:
	Operator(std::string symbol, int priority);
	std::string getSymbol();
	int getPriority();
};

