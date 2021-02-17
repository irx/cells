/*
 * TUI spreadsheet
 * 2021 Maksymilian Mruszczak <u at one u x dot o r g>
 */

#include <iostream>
#include <string>
#include <Value.h>

/**
 * Init value as `0' integer by default
 */
Value::Value(void)
{
	m_value.i = 0;
	m_type = INTEGER;
}

/**
 * Copy constructor
 */
Value::Value(const Value &v)
{
	m_type = v.m_type;
	if (m_type == STRING)
		m_value.s = new std::string(*v.m_value.s);
	else
		m_value = v.m_value;
}

/**
 * Init non-integer number
 */
Value::Value(double value)
{
	m_value.d = value;
	m_type = DOUBLE;
}

/**
 * Init integer
 */
Value::Value(int value)
{
	m_value.i = value;
	m_type = INTEGER;
}

/**
 * Init string; every value that's not a number
 * Requires additional space to be allocated.
 */
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

/**
 * Deallocation is required in case of string value
 */
Value::~Value(void)
{
	if (m_type == STRING)
		delete m_value.s;
}

/**
 * Copy value
 */
Value &
Value::operator=(const Value &v)
{
	if (m_type == STRING)
		delete m_value.s;
	m_type = v.m_type;
	if (m_type == STRING)
		m_value.s = new std::string(*v.m_value.s);
	else
		m_value = v.m_value;
	return *this;
}

/**
 * Add uint to the value;
 * needed for incrementing when inserting into
 * a range.
 */
Value
Value::operator+(unsigned ui) const
{
	switch (m_type) {
	case STRING:
		return Value(*m_value.s);
	case INTEGER:
		return Value(m_value.i + (int)ui);
	case DOUBLE:
		return Value(m_value.d + (double)ui);
	}
	return Value();
}

/**
 * Evaluate and convert into string for display
 */
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
