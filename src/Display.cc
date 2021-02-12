/* TUI spreadsheet
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
#include <Value.h>
#include <Cell.h>
#include <Sheet.h>
#include <Display.h>

static void signal_handler(int);

unsigned int Display::COLS = 0, Display::LINES = 0;

struct Display::Tty {
	struct termios orig_conf;
};

Display::Display(std::shared_ptr<Sheet> sht) : m_sheet(sht)
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
			draw_status_bar("down");
			break;
		case 'k':
			draw_status_bar("up");
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
		move(0, 0);
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

void
Display::draw_status_bar(const std::string &str)
{
	move(0, LINES - 1);
	std::string fmt = "\33[1;48;5;236;38;5;7m NORMAL \33[48;5;238;38;5;248m%" + std::to_string(COLS - 8) +"s\33[0m";
	printf(fmt.c_str(), str.c_str());
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
