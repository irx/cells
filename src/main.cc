/*
 * TUI spreadsheet
 * 2021 Maksymilian Mruszczak <u at one u x dot o r g>
 */

#include <iostream>
#include <map>
#include <memory>
#include <Value.h>
#include <Cell.h>
#include <Sheet.h>
#include <Display.h>

int
main(int argc, char *argv[])
{
	auto sheet = std::make_shared<Sheet>();
	Display d(sheet);
	if (argc > 1) {
		d.set_sheet_filename(argv[1]);
		d.load_sheet();
	}
	d.take_input();
	return 0;
}
