#include "debug_printer.h"
#include <string>
using namespace std;
#include <stdio.h>
#include <stdarg.h>

#define PRINT_DEBUG  1

void __print_msg(const char* fmt, va_list args)
{
#if (PRINT_DEBUG == 1)
	vprintf(fmt, args);
	printf("\n");
#endif
}

void debug_printer::print_msg(const char* fmt, ...)
{
	va_list argp;
	va_start(argp, fmt);

	string str_fmt;
	str_fmt = string("\033[40;32m") + string(fmt) + string("\033[0m");
	__print_msg(str_fmt.c_str(), argp);

	va_end(argp);
}

void debug_printer::print_err(const char* fmt, ...)
{
	string str_fmt;
	str_fmt = string("\033[40;31mError: ") + string(fmt) + string("\033[0m");

	va_list argp;
	va_start(argp, fmt);

	__print_msg(str_fmt.c_str(), argp);

	va_end(argp);

}
