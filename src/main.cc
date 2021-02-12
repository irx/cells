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
main(void)
{
	auto sheet = std::make_shared<Sheet>();
	Display d(sheet);
	d.take_input();
	return 0;
}
