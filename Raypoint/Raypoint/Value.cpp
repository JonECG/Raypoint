#include "Value.h"
#include <ostream>

Value::Value()
{
	//no val
	this->num = -12398;
}

Value::Value(float val)
{
	this->num = val;
}

std::string Value::toString() const
{
	return std::to_string( num );
}

ostream& operator<<(ostream& os, const Value& val)
{
	//os << val.toString();
	return os;
}