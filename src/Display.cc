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

unsigned int Display::COLS = 0, Display::LINES = 0;

struct Display::Tty {
	struct termios orig_conf;
};

Display::Display(std::shared_ptr<Sheet> sht) : m_sheet(sht), m_cursor("A1:A1")
{
	m_tty = std::make_unique<Tty>();
	printf("\33[?1049h"); /* save screen */
	fflush(stdout);
	tcgetattr(STDIN_FILENO, &m_tty->orig_conf);
	/* place for initialisations and stuff */
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sigaddset(&sa.sa_mask, SIGWINCH);
	//std::function<void(*)(int)> handler{ std::bind(&Display::signal_handler, this, std::placeholders::_1) };
	//sa.sa_handler = (void (*)(int))handler;
	//sa.sa_handler = (void (*)(int))std::bind(&Display::signal_handler, this, std::placeholders::_1);
	sa.sa_handler = signal_handler;
	if (sigaction(SIGWINCH, &sa, NULL) == -1)
		throw std::runtime_error("failed initialising signal handling");
	update_win_size();
	update_view();
	set_raw();
}

Display::~Display(void)
{
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &m_tty->orig_conf);
	printf("\33[?1049l");
	fflush(stdout);
}

void
Display::take_input(void)
{
	clear();
	draw_status_bar("Hello!");
	move(0, 0);
	fflush(stdout);
	char c;
	while (read(STDIN_FILENO, &c, 1) /*== 1*/ && c != 'q') {
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
			clear();
			draw_status_bar();
			break;
		default:
			draw_status_bar();
		}
		draw_cells();
		auto curp = get_disp_pos(m_cursor.end);
		move(curp.first, curp.second); /* jump to cursor (selection) end postion */
		fflush(stdout);
	}
}

void
Display::take_cmd(void)
{
	set_cooked();
	move(0, LINES);
	printf(":");
	fflush(stdout);
	std::string cmd;
	std::cin >> cmd;
	set_raw();
}

void
Display::clear(void)
{
	printf("\33[2J");
}

void
Display::move(unsigned x, unsigned y)
{
	printf("\33[%u;%uH", y, x);
}

void
Display::set_raw(void)
{
	struct termios raw;
	tcgetattr(STDIN_FILENO, &raw);
	raw.c_lflag &= ~(ECHO | ICANON);
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void
Display::set_cooked(void)
{
	struct termios cooked;
	tcgetattr(STDIN_FILENO, &cooked);
	cooked.c_lflag |= (ECHO | ICANON);
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &cooked);
}

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

void
Display::draw_status_bar(const std::string &str)
{
	move(0, LINES - 1);
	std::string fmt = "\33[1;48;5;236;38;5;7m NORMAL \33[48;5;238;38;5;248m%" + std::to_string(COLS - 8) +"s\33[0m";
	printf(fmt.c_str(), str.c_str());
	draw_margins(); // TMP
}

void
Display::draw_cell(const std::string &s, unsigned l, bool highlight, int fg, int bg)
{
	std::string fgs = (fg < 0) ? "" : ";38;5;" + std::to_string(fg);
	std::string bgs = (bg < 0) ? "" : ";48;5;" + std::to_string(bg);
	std::string inv = highlight ? ";7" : "";
	std::string fmt = "\33[" + fgs + bgs + inv + "m%" + std::to_string(l) + "s\33[0m";
	printf(fmt.c_str(), s.c_str());
}

void
Display::draw_margins(void)
{
	Cell::Pos p;
	move(6, 0);
	for (p.col = m_view.begin.col; p.col < m_view.end.col; ++p.col)
		draw_cell(p.get_col_str(), m_sheet->get_col_siz(p.col), false, MARGIN_FG, MARGIN_BG);
	move(0, 2);
	for (p.row = m_view.begin.row; p.row < m_view.end.row; ++p.row) {
		draw_cell(std::to_string(p.row), 5, false, MARGIN_FG, MARGIN_BG);
		printf("\n");
	}
}

void
Display::draw_cells(void)
{
	/* first draw cursor range */
	for (Cell::Pos cur = m_cursor.begin; cur.col <= m_cursor.end.col; ++cur.col)
		for (cur.row = m_cursor.begin.row; cur.row <= m_cursor.end.row; ++cur.row)
			if (m_view.contains(cur)) {
				auto absp = get_disp_pos(cur); /* translate cell addr to coord */
				move(absp.first, absp.second);
				draw_cell(cur.get_addr(), m_sheet->get_col_siz(cur.col), true);
			}
	/* draw cells with values */
	auto cells = m_sheet->get_cells(m_view);
	for (auto &c : cells) {
		auto p = c.get_pos();
		auto absp = get_disp_pos(p); /* get absolute coordinates */
		move(absp.first, absp.second);
		draw_cell(c.get_value().eval(), m_sheet->get_col_siz(p.col), m_cursor.contains(p));
	}
}

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

void
Display::update_win_size(void)
{
	struct winsize w;
	ioctl(0, TIOCGWINSZ, &w);
	Display::COLS = w.ws_col;
	Display::LINES = w.ws_row;
}

static void
signal_handler(int signo)
{
	Display::update_win_size();
	//if (signo == SIGWINCH && winch_custom_handler)
		//(*winch_custom_handler)();
}
