// LogRTF.cpp

#include "Log.h"
using namespace std;

enum RTFColor
{
	RTF_BLACK = 0,	// Corresponds to "\\cf0" (foreground) or "\cb0" (background),
	RTF_WHITE,		// "\\cf1"...etc
	RTF_GREY,
	RTF_RED,
	RTF_GREEN,
	RTF_BLUE,
	RTF_CYAN,
	RTF_YELLOW,
	RTF_MAGENTA,
	RTF_DARK_RED,
	RTF_DARK_GREEN,
	RTF_DARK_BLUE,
	RTF_LIGHT_RED,
	RTF_LIGHT_GREEN,
	RTF_LIGHT_BLUE,
	RTF_ORANGE,
	RTF_DARK_GREY
};

struct RTFFormat
{
	bool bold;
	RTFColor front_color;
	RTFColor back_color;
};

static RTFFormat rtf_formats[] = {
	{false, RTF_BLACK, RTF_WHITE},	// info
	{true, RTF_GREEN, RTF_WHITE},	// success
	{true, RTF_RED, RTF_WHITE},		// failed
	{true, RTF_WHITE, RTF_ORANGE},	// warn
	{true, RTF_WHITE, RTF_RED},		// error
	{true, RTF_WHITE, RTF_GREEN},	// debug0
	{true, RTF_WHITE, RTF_BLUE},	// debug1
	{true, RTF_WHITE, RTF_MAGENTA},	// debug2
	{true, RTF_WHITE, RTF_CYAN},	// debug3
	{true, RTF_WHITE, RTF_DARK_GREY},	// debug4
	{true, RTF_YELLOW, RTF_GREEN},	// debug5
	{true, RTF_YELLOW, RTF_BLUE},	// debug6
	{true, RTF_YELLOW, RTF_MAGENTA},// debug7
	{true, RTF_YELLOW, RTF_CYAN},	// debug8
	{true, RTF_YELLOW, RTF_DARK_GREY}	// debug9
};

void doRTFFormatting(string* msg, LogType type)
{
	const RTFFormat& format = rtf_formats[type];

	string str_beginning = format.bold ? "\\b" : "";
	string str_end = format.bold ? "\\b0" : "";
	char str_cfcb[64] = "";
	sprintf(str_cfcb, "\\cf%d \\cb%d\\highlight%d", int(format.front_color), int(format.back_color), int(format.back_color));

	*msg = str_beginning + str_cfcb + *msg + str_end;
}
