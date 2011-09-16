// LogRTFContents.h

#ifndef LOG_RTF_CONTENTS_H
#define LOG_RTF_CONTENTS_H

static const char* rtf_begin =

// RTF header
"{\\rtf1\\ansi\\deff0{\\fonttbl{\\f0 Courier New;}}\\fs20\n"

// Colour table
"{\\colortbl;\n"                     //Black
"\\red255\\green255\\blue255;\n"     //White
"\\red128\\green128\\blue128;\n"     //Grey
"\\red255\\green0\\blue0;\n"         //Red
"\\red0\\green128\\blue0;\n"         //Dark Green (Green is very ugly :p)
"\\red0\\green0\\blue255;\n"         //Blue
"\\red0\\green128\\blue255;\n"       //Lighter blue
"\\red255\\green255\\blue0;\n"       //Yellow
"\\red255\\green0\\blue255;\n"       //Magenta
"\\red128\\green0\\blue0;\n"         //Dark Red
"\\red0\\green128\\blue0;\n"         //re-Dark Green
"\\red0\\green0\\blue128;\n"         //Dark Blue
"\\red255\\green128\\blue128;\n"     //Light Red
"\\red128\\green255\\blue128;\n"     //Light Green
"\\red128\\green128\\blue255;\n"     //Light Blue
"\\red255\\green128\\blue0;\n"       //Orange
"\\red128\\green128\\blue128;\n"       //Orange
"}\n";

static const char* rtf_end = "}";

#endif // LOG_RTF_CONTENTS_H
