#pragma once
class ostream;
#include <string>
class Value
{
	float num;
public:
	Value();
	Value( float val);
	std::string toString() const;
	friend ostream& operator<<(ostream& os, const Value& val);
};

ostream& operator<<(ostream& os, const Value& val);

