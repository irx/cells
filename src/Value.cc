/* TUI spreadsheet
 * 2021 Maksymilian Mruszczak <u at one u x dot o r g>
 */

#include <iostream>
#include <string>
#include <Value.h>

Value::Value(void)
{
	m_value.i = 0;
	m_type = INTEGER;
}

Value::Value(double value)
{
	m_value.d = value;
	m_type = DOUBLE;
}

Value::Value(int value)
{
	m_value.i = value;
	m_type = INTEGER;
}

Value::Value(const std::string &value)
{
	m_value.s = new std::string(value);
	m_type = STRING;
}

Value::Value(const char *value)
{
	m_value.s = new std::string(value);
	m_type = STRING;
}

Value::~Value(void)
{
	if (m_type == STRING)
		delete m_value.s;
}

std::string
Value::eval(void) const
{
	switch (m_type) {
	case Type::INTEGER:
		return std::to_string(m_value.i);
	case Type::DOUBLE:
		return std::to_string(m_value.d);
	case Type::STRING:
		return *m_value.s;
	}
	return "";
}

Value::Type
Value::get_type(void) const
{
	return m_type;
}
