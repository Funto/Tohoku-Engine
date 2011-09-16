// LogTermVT100.cpp

#ifndef WIN32

#include "Log.h"
using namespace std;

enum TermAttr
{
	TERM_RESET = 0,	// "normal" mode
	TERM_BRIGHT = 1,// more luminosity for the foreground
	TERM_DIM = 2,	// less luminosity for the foreground
	TERM_UNDERLINE = 4,
	TERM_BLINK = 5,	// no difference...
	TERM_REVERSE = 7,	// reverse front and back color
};

enum TermColor
{
	TERM_BLACK = 0,
	TERM_RED,
	TERM_GREEN,
	TERM_YELLOW,
	TERM_BLUE,
	TERM_MAGENTA,
	TERM_CYAN,
	TERM_WHITE,
	TERM_NONE	// not really standard, but it works with xterm...^^
};

struct TermFormat
{
	TermAttr attr;
	TermColor front_color;
	TermColor back_color;
};

static const TermFormat term_formats[] =
{
	{TERM_RESET, TERM_NONE, TERM_NONE},			// info
	{TERM_BRIGHT, TERM_GREEN, TERM_NONE},		// success
	{TERM_BRIGHT, TERM_RED, TERM_NONE},			// failed
	{TERM_BRIGHT, TERM_WHITE, TERM_YELLOW},		// warn
	{TERM_BRIGHT, TERM_WHITE, TERM_RED},		// error
	{TERM_BRIGHT, TERM_WHITE, TERM_GREEN},		// debug0
	{TERM_BRIGHT, TERM_WHITE, TERM_BLUE},		// debug1
	{TERM_BRIGHT, TERM_WHITE, TERM_MAGENTA},	// debug2
	{TERM_BRIGHT, TERM_WHITE, TERM_CYAN},		// debug3
	{TERM_BRIGHT, TERM_WHITE, TERM_BLACK},		// debug4
	{TERM_BRIGHT, TERM_YELLOW, TERM_GREEN},		// debug5
	{TERM_BRIGHT, TERM_YELLOW, TERM_BLUE},		// debug6
	{TERM_BRIGHT, TERM_YELLOW, TERM_MAGENTA},	// debug7
	{TERM_BRIGHT, TERM_YELLOW, TERM_CYAN},		// debug8
	{TERM_BRIGHT, TERM_YELLOW, TERM_BLACK}		// debug9
};

// Useless in the vt100/xterm implementation, but necessary for the Windows version.
void writeTermFormattedString(ostream* p_stream, const string& str)
{
	(*p_stream) << str;
}

void resetTerm(ostream* p_stream)
{
	(*p_stream) << (char)0x1B << "[0;;m";
}

void doTermFormatting(string* msg, LogType type)
{
	const TermFormat& format = term_formats[type];

	char str_beginning[64] = "";
	sprintf(str_beginning, "%c[%d;%d;%dm",
		0x1B,
		int(format.attr),
		int(format.front_color)+30,
		int(format.back_color)+40);
	*msg = string(str_beginning) + *msg;
}
#endif // !defined WIN32

