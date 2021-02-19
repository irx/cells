/*
 * TUI spreadsheet
 * 2021 Maksymilian Mruszczak <u at one u x dot o r g>
 */

#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include <Value.h>
#include <Cell.h>
#include <Sheet.h>

#define DEFAULT_WIDTH 10
#define DEFAULT_HEIGHT 1

Sheet::Sheet(void)
{}

Sheet::~Sheet(void)
{}

/**
 * Insert value[s] into given cell range of the sheet
 * For a given cell value is increased by its index
 * i.e. distance from the starting point.
 */
void
Sheet::insert(const Cell::Range &range, const Value &value)
{
	for (Cell::Pos cur = range.begin; cur.col <= range.end.col; ++cur.col)
		for (cur.row = range.begin.row; cur.row <= range.end.row; ++cur.row)
			m_cells[cur] = Cell(cur, value + range.index_of(cur));
}

/**
 * Remove values in a given range
 */
void
Sheet::remove(const Cell::Range &range)
{
	for (Cell::Pos cur = range.begin; cur.col <= range.end.col; ++cur.col)
		for (cur.row = range.begin.row; cur.row <= range.end.row; ++cur.row)
			m_cells.erase(cur);
}

/**
 * Parse input value;
 * convert it either to int, double
 * or leave it as a string.
 */
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

/**
 * Get cells from a given range
 */
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

/**
 * Get width of a column
 */
unsigned
Sheet::get_col_siz(unsigned idx) const
{
	return m_col_siz.count(idx) > 0 ? m_col_siz.at(idx) : DEFAULT_WIDTH;
}

/**
 * Get height of a row
 */
unsigned
Sheet::get_row_siz(unsigned idx) const
{
	return m_row_siz.count(idx) > 0 ? m_row_siz.at(idx) : DEFAULT_HEIGHT;
}

/**
 * Set width of a column
 */
void
Sheet::set_col_siz(unsigned idx, unsigned siz)
{
	m_col_siz[idx] = siz;
}

/**
 * Set height of a row
 */
void
Sheet::set_row_siz(unsigned idx, unsigned siz)
{
	m_row_siz[idx] = siz;
}

/**
 * Increase width of a column by 1
 */
void
Sheet::increase_col_siz(unsigned idx)
{
	if (m_col_siz.count(idx) < 1)
		m_col_siz[idx] = DEFAULT_WIDTH;
	++m_col_siz[idx];
}

/**
 * Decrease width of a column by 1
 */
void
Sheet::decrease_col_siz(unsigned idx)
{
	if (m_col_siz.count(idx) < 1)
		m_col_siz[idx] = DEFAULT_WIDTH;
	--m_col_siz[idx];
}

/**
 * Translate cell address into terminal coordinates
 */
std::pair<unsigned, unsigned>
Sheet::get_abs_pos(const Cell::Pos &p) const
{
	unsigned c, r, i;
	c = r = 0;
	for (i = 1; i < p.col; ++i)
		c += get_col_siz(i);
	for (i = 1; i < p.row; ++i)
		r += get_row_siz(i);
	return std::make_pair(c, r);
}

/**
 * Open a sheet file
 */
void
Sheet::load(const std::string &filename)
{
	std::fstream fs(filename, std::fstream::in);
	std::string ln, tk; /* line, token */
	size_t pos, pos2; /* delimiter position */
	std::getline(fs, ln);
	if (ln != "CELLSF") /* magic sequence; basic sanity check */
		throw std::runtime_error("invalid file type");
	/* read column sizes */
	std::getline(fs, ln);
	for (pos = 0; (pos = ln.find(";")) != std::string::npos && !ln.empty(); ln.erase(0, pos + 1)) {
		tk = ln.substr(0, pos);
		pos2 = tk.find(":");
		m_col_siz[(unsigned)std::stoi(tk.substr(0, pos2))] = (unsigned)std::stoi(tk.substr(pos2 + 1));
	}
	/* read row sizes */
	std::getline(fs, ln);
	for (pos = 0; (pos = ln.find(";")) != std::string::npos && !ln.empty(); ln.erase(0, pos + 1)) {
		tk = ln.substr(0, pos);
		pos2 = tk.find(":");
		m_row_siz[(unsigned)std::stoi(tk.substr(0, pos2))] = (unsigned)std::stoi(tk.substr(pos2 + 1));
	}
	/* read cell contents */
	while (std::getline(fs, ln)) {
		pos = ln.find(";");
		tk = ln.substr(0, pos);
		Cell::Pos p(tk);
		tk = ln.substr(pos + 1);
		m_cells[p] = Cell(p, parse(tk));
	}
}

/* Save the sheet into a file */
void
Sheet::save(const std::string &filename) const
{
	std::fstream fs(filename, std::fstream::out);
	fs << "CELLSF\n";
	/* write column sizes */
	for (auto &c : m_col_siz)
		fs << c.first << ":" << c.second << ";";
	fs << '\n';
	/* write row sizes */
	for (auto &c : m_row_siz)
		fs << c.first << ":" << c.second << ";";
	fs << '\n';
	/* write cell contents */
	for (auto &c : m_cells) {
		auto v = c.second.get_value();
		fs << c.first.get_addr() << ";" << v->eval() << '\n';
	}
}
