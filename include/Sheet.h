/* TUI spreadsheet
 * 2021 Maksymilian Mruszczak <u at one u x dot o r g>
 */

class Sheet
{
	public:
	Sheet(void);
	~Sheet(void);

	void insert(Cell::Pos, Value);

	private:
	std::vector<unsigned> m_col_siz, m_row_siz;
	std::map<Cell::Pos, Cell> m_cells;
	Cell::Pos m_selected;
};
