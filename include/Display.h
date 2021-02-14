/*
 * TUI spreadsheet
 * 2021 Maksymilian Mruszczak <u at one u x dot o r g>
 */

class Display
{
	public:
	enum Mode {
		NORMAL,
		INPUT,
		COMMAND
	};

	Display(std::shared_ptr<Sheet>);
	~Display(void);

	void take_cmd(void);
	void take_input(void);
	void take_value(void);
	void refresh(void);

	static void update_win_size(void);
	static unsigned int COLS, LINES;

	private:
	struct Tty;

	void clear(void);
	void move(unsigned, unsigned);
	void set_raw(void);
	void set_cooked(void);
	void update_view(void);
	void update_hview(void); /* update horizontal view */
	void update_vview(void); /* update vertical view */
	void print_err(const char *);

	void set_sheet_filename(const std::string &);
	void save_sheet(void);
	void load_sheet(void);

	std::pair<unsigned, unsigned> get_disp_pos(const Cell::Pos &) const;
	void draw_status_bar(const std::string &str = "");
	void draw_cell(const std::string &s, unsigned l, bool highlight = false, bool fill = true, int fg = -1, int bg = -1);
	void draw_margins(void);
	void draw_cells(void); /* draw all the cells within view range */

	std::unique_ptr<Tty> m_tty;
	std::shared_ptr<Sheet> m_sheet;
	Cell::Range m_view, m_cursor;
	std::string m_filename;
	Mode m_mode;
};
