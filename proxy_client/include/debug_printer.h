#ifndef debug_printer_h__
#define debug_printer_h__

#include <iosfwd>
using namespace std;
class debug_printer
{
public:
	static void print_msg(const char* fmt, ...);
	static void print_err(const char* fmt, ...);
};

#define DEBUG_MSG(logs) \
	do \
	{\
		ostringstream var;\
		var << "[" << __FILE__ << ":" << __LINE__ << "] - " <<logs;\
		debug_printer::print_msg(var.str().c_str());\
	}while (0)

#define DEBUG_ERROR(logs) \
	do \
	{\
		ostringstream var;\
		var << "[" << __FILE__ << ":" << __LINE__ << "] - " <<logs;\
		debug_printer::print_err(var.str().c_str());\
	}while (0)


#endif // debug_printer_h__
