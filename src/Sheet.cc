/* TUI spreadsheet
 * 2021 Maksymilian Mruszczak <u at one u x dot o r g>
 */

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <Value.h>
#include <Cell.h>
#include <Sheet.h>

#define DEFAULT_WIDTH 10
#define DEFAULT_HEIGHT 1

Sheet::Sheet(void)
{}

void
Sheet::insert(Cell::Pos pos, Value value)
{}

Value
Sheet::parse(const std::string &s)
{
	bool frac = false;
	for (auto &c : s)
		if (!std::isdigit(c)) {
			if (c == '.' && !frac)
				frac = true;
			else
				return Value(s);
		}
	return frac ? Value(std::stod(s)) : Value(std::stoi(s));
}

std::vector<Cell>
Sheet::get_cells(const Cell::Range &r) const
{
	std::vector<Cell> cells;
	if (r.end < r.begin)
		return cells; /*should probably rise exception */
	/* get lower and upper bound map iterators */
	auto itl = m_cells.lower_bound(r.begin);
	auto itu = m_cells.upper_bound(r.end);
	for (; itl != itu; ++itl)
		cells.push_back(itl->second);
	return cells;
}

unsigned
Sheet::get_col_siz(unsigned idx) const
{
	return m_col_siz.count(idx) > 0 ? m_col_siz.at(idx) : DEFAULT_WIDTH;
}

unsigned
Sheet::get_row_siz(unsigned idx) const
{
	return m_row_siz.count(idx) > 0 ? m_row_siz.at(idx) : DEFAULT_HEIGHT;
}
