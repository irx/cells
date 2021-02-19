/*
 * TUI spreadsheet
 * 2021 Maksymilian Mruszczak <u at one u x dot o r g>
 *
 * This class stores arbitrary variable.
 * There are free possible types;
 * anything that's not an integer or decimal (double)
 * is stored as a string.
 */

class Value
{
	public:
	enum Type {
		INTEGER,
		DOUBLE,
		STRING
	};
	Value(void);
	Value(const Value &);
	Value(double);
	Value(int);
	Value(const std::string &);
	Value(const char *);
	~Value(void);

	Value &operator=(const Value &);
	Value operator+(unsigned) const;
	std::string eval(void) const;
	Type get_type(void) const;

	private:
	union _Value {
		int i;
		double d;
		std::string *s;
	};
	_Value m_value;
	Type m_type;
};
