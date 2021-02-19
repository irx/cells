/*
 * TUI spreadsheet
 * 2021 Maksymilian Mruszczak <u at one u x dot o r g>
 *
 * This class manages spreadheet data.
 * Cells and spreadsheet dimensions are stored in a map
 * so the address of a given cell is unconstrained.
 * Arbitrary string can be converted to adequate value type
 * by using parse method.
 */

class Sheet
{
	public:
	Sheet(void);
	~Sheet(void);

	void insert(const Cell::Range &, const Value &);
	void remove(const Cell::Range &);
	Value parse(const std::string &);
	std::vector<Cell> get_cells(const Cell::Range &) const;
	unsigned get_col_siz(unsigned) const;
	unsigned get_row_siz(unsigned) const;
	void set_col_siz(unsigned, unsigned);
	void set_row_siz(unsigned, unsigned);
	void increase_col_siz(unsigned);
	void decrease_col_siz(unsigned);
	std::pair<unsigned, unsigned> get_abs_pos(const Cell::Pos &) const;
	void load(const std::string &);
	void save(const std::string &) const;

	private:
	std::map<unsigned, unsigned> m_col_siz, m_row_siz;
	std::map<Cell::Pos, Cell> m_cells;
};
