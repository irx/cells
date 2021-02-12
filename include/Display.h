/*
 * TUI spreadsheet
 * 2021 Maksymilian Mruszczak <u at one u x dot o r g>
 */

class Display
{
public:
	Display(std::shared_ptr<Sheet>);
	~Display(void);

	void take_cmd(void);
	void take_input(void);
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

	void draw_status_bar(const std::string &str = "");

	std::unique_ptr<Tty> m_tty;
	std::shared_ptr<Sheet> m_sheet;
	Cell::Range m_view, m_cursor;
};
