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

Sheet::Sheet(void)
{}

void
Sheet::insert(Cell::Pos pos, Value value)
{}

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
