/* TUI spreadsheet
 * 2021 Maksymilian Mruszczak <u at one u x dot o r g>
 */

#include <string>
#include <vector>
#include <Value.h>
#include <Cell.h>

Cell::Cell(Value v)
{
	m_value = v;
}

bool
Cell::Pos::operator<(const Pos &p) const
{
	return (row < p.row && col < p.col);
}
