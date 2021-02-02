/* TUI spreadsheet
 * 2021 Maksymilian Mruszczak <u at one u x dot o r g>
 */

class Cell
{
	public:
	struct Pos {
		unsigned row, col;
		bool operator<(const Pos &) const;
	};

	Cell(Value);

	private:
	Value m_value;
	Pos m_pos;
};
