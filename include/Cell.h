/**
 * TUI spreadsheet
 * 2021 Maksymilian Mruszczak <u at one u x dot o r g>
 */

class Cell
{
	public:
	struct Pos {
		struct address_error : public std::runtime_error {
			address_error(const std::string &);
		};

		unsigned row, col;
		bool row_iter, col_iter; /* is iterable */
		Pos(void);
		Pos(const std::string &);
		bool operator<(const Pos &) const;
		bool operator==(const Pos &) const;
		std::string get_col_str(void) const;
		std::string get_addr(void) const;
	};
	struct Range {
		Pos begin, end;

		Range(void);
		Range(Pos, Pos);
		Range(const std::string &);
		std::string get_addr(void) const;
	};

	Cell(Value);

	private:
	Value m_value;
	Pos m_pos;
};
