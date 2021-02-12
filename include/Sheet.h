/* TUI spreadsheet
 * 2021 Maksymilian Mruszczak <u at one u x dot o r g>
 */

class Sheet
{
	public:
	Sheet(void);
	~Sheet(void);

	void insert(Cell::Pos, Value);
	Value parse(const std::string &);
	std::vector<Cell> get_cells(const Cell::Range &) const;
	unsigned get_col_siz(unsigned) const;
	unsigned get_row_siz(unsigned) const;

	private:
	std::map<unsigned, unsigned> m_col_siz, m_row_siz;
	std::map<Cell::Pos, Cell> m_cells;
	Cell::Pos m_selected;
};
