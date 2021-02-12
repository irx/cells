/* TUI spreadsheet
 * 2021 Maksymilian Mruszczak <u at one u x dot o r g>
 */

class Display
{
public:
	Display(void);
	~Display(void);

	void take_cmd(void);
	void take_input(void);
	void refresh(void);

	static void update_win_size(void);
	static unsigned int COLS, LINES;

private:
	struct Tty;

	void clear();
	void move(unsigned, unsigned);
	void set_raw(void);
	void set_cooked(void);

	void draw_status_bar(const std::string &str = "");

	std::unique_ptr<Tty> m_tty;
	Cell::Range m_view;
};
