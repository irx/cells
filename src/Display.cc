/*
 * TUI spreadsheet
 * 2021 Maksymilian Mruszczak <u at one u x dot o r g>
 */

#include <unistd.h>
#include <cstdlib>
#include <signal.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>

#include <functional>
#include <map>
#include <memory>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <Value.h>
#include <Cell.h>
#include <Sheet.h>
#include <Display.h>

#define MARGIN_FG 244
#define MARGIN_BG 232

static void signal_handler(int);

static const char *mode_str[] = {
	"NORMAL",
	"INPUT",
	"COMMAND"
};

unsigned int Display::COLS = 0, Display::LINES = 0;

struct Display::Tty {
	struct termios orig_conf;
};

/**
 * Display constructor
 * A terminal config struct is initialised
 * along with signal handler which is used
 * to trigger column/row count reeevaluation
 * when screen size is changed.
 */
Display::Display(std::shared_ptr<Sheet> sht) : m_sheet(sht), m_cursor("A1:A1"), m_mode(NORMAL)
{
	m_tty = std::make_unique<Tty>();
	printf("\33[?1049h"); /* save screen */
	fflush(stdout);
	tcgetattr(STDIN_FILENO, &m_tty->orig_conf);
	/* place for initialisations and stuff */
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sigaddset(&sa.sa_mask, SIGWINCH);
	sa.sa_handler = signal_handler;
	if (sigaction(SIGWINCH, &sa, NULL) == -1)
		throw std::runtime_error("failed initialising signal handling");
	update_win_size();
	update_view();
	set_raw();
}

/**
 * Display destructor
 * Restore orignal terminal config and contents
 */
Display::~Display(void)
{
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &m_tty->orig_conf);
	printf("\33[?1049l"); /* restore terminal content */
	fflush(stdout);
}

/**
 * Take input from user while in raw terminal mode.
 * Fetch char after every key stroke and interpret
 * it as a interactive (NORMAL) mode command.
 */
void
Display::take_input(void)
{
	m_taking_input = true;
	clear();
	draw_status_bar("Hello!");
	draw_margins();
	draw_cells();
	auto curp = get_disp_pos(m_cursor.end);
	move(curp.first, curp.second);
	fflush(stdout);
	char c;
	while (m_taking_input && read(STDIN_FILENO, &c, 1)) {
		clear();
		switch (c) {
		case 'j':
			++m_cursor.end.row;
			m_cursor.begin = m_cursor.end;
			update_vview();
			draw_status_bar("down");
			break;
		case 'k':
			if (m_cursor.end.row > 1)
				--m_cursor.end.row;
			m_cursor.begin = m_cursor.end;
			update_vview();
			draw_status_bar("up");
			break;
		case 'l':
			++m_cursor.end.col;
			m_cursor.begin = m_cursor.end;
			update_hview();
			draw_status_bar("right");
			break;
		case 'h':
			if (m_cursor.end.col > 1)
				--m_cursor.end.col;
			m_cursor.begin = m_cursor.end;
			update_hview();
			draw_status_bar("left");
			break;
		case 'g':
			m_cursor = Cell::Range("A1:A1");
			update_view();
			draw_status_bar("go to top");
			break;
		case 'J':
			++m_cursor.end.row;
			update_vview();
			draw_status_bar("extend vertical");
			break;
		case 'K':
			if (m_cursor.end.row > m_cursor.begin.row)
				--m_cursor.end.row;
			update_vview();
			draw_status_bar("retract vertical");
			break;
		case 'L':
			++m_cursor.end.col;
			update_hview();
			draw_status_bar("extend horizontal");
			break;
		case 'H':
			if (m_cursor.end.col > m_cursor.begin.col)
				--m_cursor.end.col;
			update_hview();
			draw_status_bar("retract horizontal");
			break;
		case 'G':
			m_cursor.begin = m_cursor.end = m_view.end;
			draw_status_bar("go to bottom edge");
			break;
		case ':':
			draw_status_bar();
			take_cmd();
			break;
		case 'i':
			take_value();
			clear();
			draw_status_bar();
			break;
		case 'd':
			m_sheet->remove(m_cursor);
			clear();
			draw_status_bar("remove");
			break;
		case '+':
			m_sheet->increase_col_siz(m_cursor.end.col);
			update_hview();
			break;
		case '-':
			m_sheet->decrease_col_siz(m_cursor.end.col);
			update_hview();
			break;
		default:
			draw_status_bar();
		}
		draw_margins();
		draw_cells();
		auto curp = get_disp_pos(m_cursor.end);
		move(curp.first, curp.second); /* jump to cursor (selection) end postion */
		fflush(stdout);
	}
}

/**
 * Enter command mode
 * Set terminal to cooked mode and
 * take whole line of input.
 * Allows entering commands wih arguments.
 */
void
Display::take_cmd(void)
{
	m_mode = COMMAND;
	draw_status_bar("command");
	draw_margins();
	draw_cells();
	set_cooked();
	move(0, LINES);
	printf(":");
	fflush(stdout);
	std::string cmd;
	std::cin >> cmd;
	if (cmd == "f") {
		std::cin >> cmd;
		set_sheet_filename(cmd);
	} else if (cmd == "w")
		save_sheet();
	else if (cmd == "r")
		load_sheet();
	else if (cmd == "q")
		m_taking_input = false;
	else
		print_err("unrecognised command");
	set_raw();
	m_mode = NORMAL;
}

/**
 * Take new cell value
 * Enter cooked terminal mode and
 * take a line of text to be interpreted as
 * either string or number and put it into
 * selected cell[s]
 */
void
Display::take_value(void)
{
	m_mode = INPUT;
	draw_status_bar("input");
	draw_margins();
	draw_cells();
	set_cooked();
	auto curp = get_disp_pos(m_cursor.end);
	move(curp.first, curp.second);
	std::string val;
	std::getline(std::cin, val);
	m_sheet->insert(m_cursor, m_sheet->parse(val));
	set_raw();
	m_mode = NORMAL;
}

/**
 * Clear screen magical escape sequence
 */
void
Display::clear(void)
{
	printf("\33[2J");
}

/**
 * This escape sequence changes position of the cursor
 */
void
Display::move(unsigned x, unsigned y)
{
	printf("\33[%u;%uH", y, x);
}

/**
 * Set raw terminal mode
 * Turn off echo and canonical terminal flags.
 * This allows handling user input in interactive manner.
 */
void
Display::set_raw(void)
{
	struct termios raw;
	tcgetattr(STDIN_FILENO, &raw); /* load stdin config */
	raw.c_lflag &= ~(ECHO | ICANON); /* disable flags */
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw); /* put modified config in place */
}

/**
 * Set cooked terminal mode
 * Turns echo and canonical mode back on
 * Cananonical mode means that user input is retrieved
 * only after carriage return.
 */
void
Display::set_cooked(void)
{
	struct termios cooked;
	tcgetattr(STDIN_FILENO, &cooked);
	cooked.c_lflag |= (ECHO | ICANON);
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &cooked);
}

/**
 * Get position on display
 * Translate cell address to the absolute display
 * coordinates.
 */
std::pair<unsigned, unsigned>
Display::get_disp_pos(const Cell::Pos &p) const
{
	/* translate cell addr to display position */
	auto absview = m_sheet->get_abs_pos(m_view.begin);
	auto absp = m_sheet->get_abs_pos(p);
	absp.first -= absview.first - 6;
	absp.second -= absview.second - 2;
	return absp;
}

/**
 * Draw a nice status bar at the bottom of the screen.
 * ATM it shows mode and last interactive command issued.
 */
void
Display::draw_status_bar(const std::string &str)
{
	move(0, LINES - 1);
	std::string fmt = "\33[1;48;5;236;38;5;7m %7.7s \33[48;5;238;38;5;248m%" + std::to_string(COLS - 9) +"s\33[0m";
	printf(fmt.c_str(), mode_str[m_mode], str.c_str());
}

/**
 * Draw a cell on the screen
 */
void
Display::draw_cell(const std::string &s, unsigned l, bool highlight, bool fill, int fg, int bg)
{
	std::string fgs = (fg < 0) ? "" : ";38;5;" + std::to_string(fg),
	            bgs = (bg < 0) ? "" : ";48;5;" + std::to_string(bg),
	            inv = highlight ? ";7" : "",
	            len = std::to_string(l);
	len = fill ? len + "." + len : "." + len;
	std::string fmt = "\33[" + fgs + bgs + inv + "m%" + len + "s\33[0m";
	printf(fmt.c_str(), s.c_str());
}

/**
 * Draw margins that denote column and row address
 */
void
Display::draw_margins(void)
{
	Cell::Pos p;
	move(6, 0);
	for (p.col = m_view.begin.col; p.col < m_view.end.col; ++p.col)
		draw_cell(p.get_col_str(), m_sheet->get_col_siz(p.col), (p.col == m_cursor.end.col), true, MARGIN_FG, MARGIN_BG);
	move(0, 2);
	for (p.row = m_view.begin.row; p.row < m_view.end.row; ++p.row) {
		draw_cell(std::to_string(p.row), 5, (p.row == m_cursor.end.row), true, MARGIN_FG, MARGIN_BG);
		printf("\n");
	}
}

/**
 * Draw all the cells from sheet that are in visible range
 */
void
Display::draw_cells(void)
{
	/* first draw cursor range */
	for (Cell::Pos cur = m_cursor.begin; cur.col <= m_cursor.end.col; ++cur.col)
		for (cur.row = m_cursor.begin.row; cur.row <= m_cursor.end.row; ++cur.row)
			if (m_view.contains(cur)) {
				auto absp = get_disp_pos(cur); /* translate cell addr to coord */
				move(absp.first, absp.second);
				draw_cell("", m_sheet->get_col_siz(cur.col), true);
			}
	/* draw cells with values */
	auto cells = m_sheet->get_cells(m_view);
	for (auto &c : cells) {
		auto p = c.get_pos();
		if (m_view.contains(p)) {
			auto absp = get_disp_pos(p); /* get absolute coordinates */
			auto v = c.get_value();
			unsigned colour = 1;
			bool fill = true;
			if (v->get_type() == Value::Type::STRING) {
				colour = 7;
				fill = false;
			}
			move(absp.first, absp.second);
			draw_cell(v->eval(), m_sheet->get_col_siz(p.col), m_cursor.contains(p), fill, colour);
		}
	}
}

/**
 * Reset view
 * Set view in a way that all the available screen (terminal)
 * space is filled by cells and cursor ends up at the upper-left
 * corner of the visible range.
 */
void
Display::update_view(void)
{
	m_view.begin = m_view.end = m_cursor.end;
	unsigned tmp = m_sheet->get_col_siz(m_view.end.col);
	while (tmp < COLS - 6)
		tmp += m_sheet->get_col_siz(m_view.end.col++);
	tmp = m_sheet->get_row_siz(m_view.end.row);
	while (tmp < LINES - 3)
		tmp += m_sheet->get_row_siz(++m_view.end.row);
}

/**
 * Update horizontal view
 * Move view left/right to make curosor visible.
 */
void
Display::update_hview(void)
{
	if (m_view.end.col < m_cursor.end.col) {
		auto diff = m_cursor.end.col - m_view.end.col;
		m_view.end.col += diff;
		auto b = m_sheet->get_abs_pos(m_view.begin);
		auto e = m_sheet->get_abs_pos(m_view.end);
		diff = e.first - b.first;
		while (diff > COLS - 6)
			diff -= m_sheet->get_col_siz(m_view.begin.col++);
	} else if (m_view.begin.col > m_cursor.end.col) {
		auto diff = m_view.begin.col - m_cursor.end.col;
		m_view.begin.col -= diff;
		auto b = m_sheet->get_abs_pos(m_view.begin);
		auto e = m_sheet->get_abs_pos(m_view.end);
		diff = e.first - b.first;
		while (diff > COLS - 6)
			diff -= m_sheet->get_col_siz(m_view.end.col--);
	}
}

/**
 * Update vertical view
 * Move view up/down to make curosor visible.
 */
void
Display::update_vview(void)
{
	if (m_view.end.row < m_cursor.end.row) {
		auto diff = m_cursor.end.row - m_view.end.row;
		m_view.end.row += diff;
		auto b = m_sheet->get_abs_pos(m_view.begin);
		auto e = m_sheet->get_abs_pos(m_view.end);
		diff = e.second - b.second;
		while (diff > LINES - 4)
			diff -= m_sheet->get_row_siz(m_view.begin.row++);
	} else if (m_view.begin.row > m_cursor.end.row) {
		auto diff = m_view.begin.row - m_cursor.end.row;
		m_view.begin.row -= diff;
		auto b = m_sheet->get_abs_pos(m_view.begin);
		auto e = m_sheet->get_abs_pos(m_view.end);
		diff = e.second - b.second;
		while (diff > LINES - 4)
			diff -= m_sheet->get_row_siz(m_view.end.row--);
	}
}

/**
 * Print error at the bottom of the screen
 */
void
Display::print_err(const char *e)
{
	move(0, LINES);
	printf("\33[31;1merror:\33[0m %s", e);
}

/**
 * Set file name of the currently open sheet
 */
void
Display::set_sheet_filename(const std::string &filename)
{
	m_filename = filename;
	move(0, LINES); /* move to the bottom to print msg there */
	printf("filename set to \"%s\"", filename.c_str());
}

/**
 * Save currenttly open sheet
 */
void
Display::save_sheet(void)
{
	if (m_filename.empty()) {
		print_err("no filename set");
		return;
	}
	try {
		m_sheet->save(m_filename);
		move(0, LINES);
		printf("written to file \"%s\"", m_filename.c_str());
	} catch (const std::exception &e) {
		print_err(e.what());
	}
}

/**
 * Load sheet located under currently selected filename
 */
void
Display::load_sheet(void)
{
	if (m_filename.empty()) {
		print_err("no filename set");
		return;
	}
	try {
		m_sheet->load(m_filename);
		move(0, LINES);
		printf("read file \"%s\"", m_filename.c_str());
	} catch (const std::exception &e) {
		print_err(e.what());
	}
}

/**
 * Update window size
 * Retrieve column and row count of the current
 * terminal view. It is triggered by signal handler.
 */
void
Display::update_win_size(void)
{
	struct winsize w;
	ioctl(0, TIOCGWINSZ, &w);
	Display::COLS = w.ws_col;
	Display::LINES = w.ws_row;
}

/**
 * Triggers when window change signal is issued
 */
static void
signal_handler(int signo)
{
	Display::update_win_size();
}
