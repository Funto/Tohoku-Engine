// LogHTML.cpp

#include "Log.h"
using namespace std;

static const char* css_classes[] = {
	"info",
	"success",
	"failed",
	"warn",
	"error",
	"debug0",
	"debug1",
	"debug2",
	"debug3",
	"debug4",
	"debug5",
	"debug6",
	"debug7",
	"debug8",
	"debug9",
};

void doHTMLFormatting(string* msg, LogType type)
{
	const char* css_class = css_classes[type];
	*msg =	string("<span class=\"") + string(css_class) + "\">" + *msg + "</span>";
}
