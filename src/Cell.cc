/**
 * TUI spreadsheet
 * 2021 Maksymilian Mruszczak <u at one u x dot o r g>
 */

#include <stdexcept>
#include <string>
#include <vector>
#include <Value.h>
#include <Cell.h>

#define FIRST_LETTER 0x41
#define LAST_LETTER 0x5a
#define IS_LETTER(c) (c >= FIRST_LETTER && c <= LAST_LETTER)

Cell::Cell(Value v)
{
	m_value = v;
}

Cell::Pos::Pos(void) : row(0), col(0), row_iter(true), col_iter(true)
{}

Cell::Pos::Pos(const std::string &addr) : row(0), col(0), row_iter(true), col_iter(true)
{
	std::vector<char> l;
	unsigned mag;
	std::string::const_iterator it = addr.cbegin();
	if (*it == '$') {
		col_iter = false;
		++it;
	}
	for (; it != addr.cend(); ++it) {
		if (IS_LETTER(*it))
			l.push_back(*it);
		else
			break;
	}
	if (l.size() < 1)
		throw address_error(addr);
	if (*it == '$') {
		row_iter = false;
		++it; /* const */
	}
	std::string row_addr(it, addr.cend());
	try {
		row = std::stoi(row_addr);
	} catch (const std::exception &e) {
		throw address_error(addr);
	}
	for (mag = 1; !l.empty(); l.pop_back()) {
		col += mag * (l.back() + 1 - FIRST_LETTER);
		mag *= LAST_LETTER - FIRST_LETTER + 1;
	}
}

Cell::Pos::address_error::address_error(const std::string &a) : runtime_error(a + " is not a cell addr")
{}

bool
Cell::Pos::operator<(const Pos &p) const
{
	return (row < p.row && col < p.col);
}

bool
Cell::Pos::operator==(const Pos &p) const
{
	return (row == p.row && col == p.col);
}

std::string
Cell::Pos::get_addr(void) const
{
	/* convert column number to A-Z ``digits'' */
	std::string col_str;
	unsigned nsrc, src = col;
	constexpr unsigned diff = LAST_LETTER - FIRST_LETTER + 1;
	while (src > diff) {
		nsrc = --src / diff;
		col_str.insert(0, 1, (char)(src - (nsrc * diff) + FIRST_LETTER));
		src = nsrc;
	}
	col_str.insert(0, 1, (char)(src + FIRST_LETTER - 1));
	/* put address string together */
	return (col_iter ? "" : "$") + col_str + (row_iter ? "" : "$") + std::to_string(row);
}

Cell::Range::Range(void) : begin(), end()
{}

Cell::Range::Range(Pos b, Pos e) : begin(b), end(e)
{}

Cell::Range::Range(const std::string &str)
{
	auto delim = str.find(":");
	begin = Pos(str.substr(0, delim));
	end = Pos(str.substr(delim + 1));
}

std::string
Cell::Range::get_addr(void) const
{
	return begin.get_addr() + ":" + end.get_addr();
}
