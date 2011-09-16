// Log.h

#ifndef LOG_H
#define LOG_H

// If we want to specify at the compilation command line the flags
// we want to enable/disable, we need to add -DUSE_COMPILE_FLAGS,
// otherwise we use the values right here.
#ifndef USE_COMPILE_FLAGS

// Flags:
#define ENABLE_LOGGING
//#define DISABLE_DEBUG_LOGGING	// If defined, disables only logDebugXXX() macros.
//#define LOG_USE_GLEW
//#define LOG_ENABLE_GLFW_THREADS

#endif // USE_COMPILE_FLAGS

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>

#ifdef LOG_USE_GLEW
#include <GL/glew.h>
#endif

#ifdef LOG_ENABLE_GLFW_THREADS
#include <GL/glfw.h>
#endif

#ifndef ENABLE_LOGGING

inline void logInfo(...) {}
inline void logSuccess(...) {}
inline void logFailed(...) {}
inline void logWarn(...) {}
inline void logError(...) {}
inline void logDebug(...) {}
inline void logDebug0(...) {}
inline void logDebug1(...) {}
inline void logDebug2(...) {}
inline void logDebug3(...) {}
inline void logDebug4(...) {}
inline void logDebug5(...) {}
inline void logDebug6(...) {}
inline void logDebug7(...) {}
inline void logDebug8(...) {}
inline void logDebug9(...) {}

#else	// if ENABLE_LOGGING is enabled:

#define logInfo(...)		Log::write(LOG_INFO,	__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define logSuccess(...)		Log::write(LOG_SUCCESS,	__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define logFailed(...)		Log::write(LOG_FAILED,	__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define logWarn(...)		Log::write(LOG_WARN,	__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define logError(...)		Log::write(LOG_ERROR,	__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)

#ifndef DISABLE_DEBUG_LOGGING

#define logDebug(...)		Log::write(LOG_DEBUG,	__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define logDebug0(...)		Log::write(LOG_DEBUG_0,	__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define logDebug1(...)		Log::write(LOG_DEBUG_1,	__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define logDebug2(...)		Log::write(LOG_DEBUG_2,	__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define logDebug3(...)		Log::write(LOG_DEBUG_3,	__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define logDebug4(...)		Log::write(LOG_DEBUG_4,	__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define logDebug5(...)		Log::write(LOG_DEBUG_5,	__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define logDebug6(...)		Log::write(LOG_DEBUG_6,	__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define logDebug7(...)		Log::write(LOG_DEBUG_7,	__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define logDebug8(...)		Log::write(LOG_DEBUG_8,	__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define logDebug9(...)		Log::write(LOG_DEBUG_9,	__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)

#else // if DISABLE_DEBUG_LOGGING is defined:

#define logDebug(...)		Log::write(LOG_DEBUG,	__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
inline void logDebug0(...) {}
inline void logDebug1(...) {}
inline void logDebug2(...) {}
inline void logDebug3(...) {}
inline void logDebug4(...) {}
inline void logDebug5(...) {}
inline void logDebug6(...) {}
inline void logDebug7(...) {}
inline void logDebug8(...) {}
inline void logDebug9(...) {}

#endif

// Utility retrieve the file name from the complete path, i.e. "ls" from "/bin/ls"
static inline const char* getFileNameFromPath(const char* file_path)
{
	const char* p = &file_path[0];
	for(int i=0 ; file_path[i] != '\0' ; i++)
		if(file_path[i] == '/' || file_path[i] == '\\')
			p = &file_path[i+1];
	return p;
}

enum LogType
{
	LOG_INFO,
	LOG_SUCCESS,
	LOG_FAILED,
	LOG_WARN,
	LOG_ERROR,
	LOG_DEBUG,
	LOG_DEBUG_0=LOG_DEBUG,
	LOG_DEBUG_1,
	LOG_DEBUG_2,
	LOG_DEBUG_3,
	LOG_DEBUG_4,
	LOG_DEBUG_5,
	LOG_DEBUG_6,
	LOG_DEBUG_7,
	LOG_DEBUG_8,
	LOG_DEBUG_9
};

// Functions implemented in Log[TermWindows/VT100]/HTML/RTF.cpp
// Terminal:
void writeTermFormattedString(std::ostream* p_stream, const std::string& str);
void resetTerm(std::ostream* p_stream);
void doTermFormatting(std::string* msg, LogType type);

// HTML:
void doHTMLFormatting(std::string* msg, LogType type);

// RTF:
void doRTFFormatting(std::string* msg, LogType type);

#endif	// ENABLE_LOGGING

class Log
{
public:
	enum Output
	{
		TERMINAL,
		TERMINAL_NO_COLOR,
		TXT,
		HTML,
		RTF
	};

private:
	// Information specific to a thread
	struct ThreadInfo
	{
		std::string name;
		int indent;

		ThreadInfo() : name(""), indent(0) {}
	};

private:
	static Output output;
	static bool opened;
	static std::ostream* p_stream;
	static std::ofstream file;
#ifdef LOG_ENABLE_GLFW_THREADS
	// If we use GLFW's threading support, we manage a table of informations for each thread.
	static std::map<GLFWthread, ThreadInfo> thread_infos;
	static GLFWmutex mutex;
#else
	static int current_indent;
#endif

public:
	static void open(Output output, const std::string& filename = "", bool desync_with_stdio=true);
	static void open(int argc, char* argv[], bool desync_with_stdio=true);
	static void close();
	static void indent(int value=1);

#ifdef LOG_ENABLE_GLFW_THREADS
	static void setThreadName(const std::string& name) {thread_infos[glfwGetThreadID()].name = name;}
	static std::string getThreadName() {return thread_infos[glfwGetThreadID()].name;}
#endif

#ifndef ENABLE_LOGGING
	static void write(	LogType type,
						const char* file_path,
						int line,
						const char* function_name,
						...)
	{
	}
#else

	// "REAL" write()
	template <class T0, class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10>
	static void write(	LogType type, const char* file_path, int line, const char* function_name,
						const T0& arg0, const T1& arg1, const T2& arg2, const T3& arg3, const T4& arg4,
						const T5& arg5, const T6& arg6, const T7& arg7, const T8& arg8, const T9& arg9, const T10& arg10);

	// Wrappers aroud the "real" write()
	template <class T0>
	static void write(	LogType type, const char* file_path, int line, const char* function_name,
						const T0& arg0)
	{
		write(type, file_path, line, function_name, arg0, "", "", "", "", "", "", "", "", "", "");
	}

	template <class T0, class T1>
	static void write(	LogType type, const char* file_path, int line, const char* function_name,
											const T0& arg0, const T1& arg1)
	{
		write(type, file_path, line, function_name, arg0, arg1, "", "", "", "", "", "", "", "", "");
	}

	template <class T0, class T1, class T2>
	static void write(	LogType type, const char* file_path, int line, const char* function_name,
						const T0& arg0, const T1& arg1, const T2& arg2)
	{
		write(type, file_path, line, function_name, arg0, arg1, arg2, "", "", "", "", "", "", "", "");
	}

	template <class T0, class T1, class T2, class T3>
	static void write(	LogType type, const char* file_path, int line, const char* function_name,
						const T0& arg0, const T1& arg1, const T2& arg2, const T3& arg3)
	{
		write(type, file_path, line, function_name, arg0, arg1, arg2, arg3, "", "", "", "", "", "", "");
	}

	template <class T0, class T1, class T2, class T3, class T4>
	static void write(	LogType type, const char* file_path, int line, const char* function_name,
						const T0& arg0, const T1& arg1, const T2& arg2, const T3& arg3, const T4& arg4)
	{
		write(type, file_path, line, function_name, arg0, arg1, arg2, arg3, arg4, "", "","", "", "", "");
	}

	template <class T0, class T1, class T2, class T3, class T4, class T5>
	static void write(	LogType type, const char* file_path, int line, const char* function_name,
						const T0& arg0, const T1& arg1, const T2& arg2, const T3& arg3, const T4& arg4, const T5& arg5)
	{
		write(type, file_path, line, function_name, arg0, arg1, arg2, arg3, arg4, arg5, "", "", "", "", "");
	}

	template <class T0, class T1, class T2, class T3, class T4, class T5, class T6>
	static void write(	LogType type, const char* file_path, int line, const char* function_name,
						const T0& arg0, const T1& arg1, const T2& arg2, const T3& arg3, const T4& arg4, const T5& arg5,
						const T6& arg6)
	{
		write(type, file_path, line, function_name, arg0, arg1, arg2, arg3, arg4, arg5, arg6, "", "", "", "");
	}

	template <class T0, class T1, class T2, class T3, class T4, class T5, class T6, class T7>
	static void write(	LogType type, const char* file_path, int line, const char* function_name,
						const T0& arg0, const T1& arg1, const T2& arg2, const T3& arg3, const T4& arg4, const T5& arg5,
						const T6& arg6, const T7& arg7)
	{
		write(type, file_path, line, function_name, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, "", "", "");
	}

	template <class T0, class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8>
	static void write(	LogType type, const char* file_path, int line, const char* function_name,
						const T0& arg0, const T1& arg1, const T2& arg2, const T3& arg3, const T4& arg4, const T5& arg5,
						const T6& arg6, const T7& arg7, const T8& arg8)
	{
		write(type, file_path, line, function_name, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, "", "");
	}

	template <class T0, class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9>
	static void write(	LogType type, const char* file_path, int line, const char* function_name,
						const T0& arg0, const T1& arg1, const T2& arg2, const T3& arg3, const T4& arg4, const T5& arg5,
						const T6& arg6, const T7& arg7, const T8& arg8, const T9& arg9)
	{
		write(type, file_path, line, function_name, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, "");
	}
#endif
};

// RAII used to control indentation
struct LogIndent
{
	LogIndent() {Log::indent(1);}
	~LogIndent() {Log::indent(-1);}
};

// Handy function, wrapper for all doXXXFormatting() ones
static inline void doFormatting(std::string* msg, LogType type, Log::Output output)
{
	if(output == Log::TERMINAL)
		doTermFormatting(msg, type);
	else if(output == Log::HTML)
		doHTMLFormatting(msg, type);
	else if(output == Log::RTF)
		doRTFFormatting(msg, type);
}

// write() implementation:
template <class T0, class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10>
void Log::write(	LogType type, const char* file_path, int line, const char* function_name,
					const T0& arg0, const T1& arg1, const T2& arg2, const T3& arg3, const T4& arg4,
					const T5& arg5, const T6& arg6, const T7& arg7, const T8& arg8, const T9& arg9, const T10& arg10)
{
	// If the log has not been previously opened, we open it as a color terminal log.
	if(!opened)
		open(TERMINAL);

#ifdef LOG_ENABLE_GLFW_THREADS
	glfwLockMutex(Log::mutex);

	// We look for the current indentation
	int current_indent;
	std::map<GLFWthread, ThreadInfo>::iterator it = thread_infos.find(glfwGetThreadID());
	if(it == thread_infos.end())	// If not found, current indent is 0
		current_indent = 0;
	else
		current_indent = it->second.indent;
#endif

	// Get the file's name from its path
	const char* file_name = getFileNameFromPath(file_path);

	std::stringstream ss;
	std::string msg_first_part = "";
	std::string msg_second_part = "";
	std::string str_temp;

	// According to the type of message, prepare the formatted strings msg_first_part and msg_second_part which
	// will be written
	switch(type)
	{
	case LOG_INFO:
		ss	<< "[" << file_name << ":" << line << " in " << function_name << "]: "
			<< arg0 << arg1 << arg2 << arg3 << arg4 << arg5 << arg6 << arg7 << arg8 << arg9 << arg10 << std::flush;
		msg_first_part = ss.str();
		doFormatting(&msg_first_part, LOG_INFO, Log::output);
		break;
	case LOG_SUCCESS:
		ss	<< "[" << file_name << ":" << line << " in " << function_name << "]: "
				<< arg0 << arg1 << arg2 << arg3 << arg4 << arg5 << arg6 << arg7 << arg8 << arg9 << arg10
				<< ": " << std::flush;
		msg_first_part = ss.str();
		doFormatting(&msg_first_part, LOG_INFO, Log::output);

		msg_second_part = "[OK]";
		doFormatting(&msg_second_part, LOG_SUCCESS, Log::output);
		break;
	case LOG_FAILED:
		ss	<< "[" << file_name << ":" << line << " in " << function_name << "]: "
				<< arg0 << arg1 << arg2 << arg3 << arg4 << arg5 << arg6 << arg7 << arg8 << arg9 << arg10
				<< ": " << std::flush;
		msg_first_part = ss.str();
		doFormatting(&msg_first_part, LOG_INFO, Log::output);

		msg_second_part = "[FAILED]";
		doFormatting(&msg_second_part, LOG_FAILED, Log::output);
		break;
	case LOG_WARN:
		ss	<< "[" << file_name << ":" << line << " in " << function_name << "]: WARNING: "
			<< arg0 << arg1 << arg2 << arg3 << arg4 << arg5 << arg6 << arg7 << arg8 << arg9 << arg10 << std::flush;
		msg_first_part = ss.str();
		doFormatting(&msg_first_part, LOG_WARN, Log::output);
		break;
	case LOG_ERROR:
		ss	<< "[" << file_name << ":" << line << " in " << function_name << "]: ERROR: "
			<< arg0 << arg1 << arg2 << arg3 << arg4 << arg5 << arg6 << arg7 << arg8 << arg9 << arg10 << std::flush;
		msg_first_part = ss.str();
		doFormatting(&msg_first_part, LOG_ERROR, Log::output);
		break;

	default:	// LOG_DEBUG_X
		ss	<< "[" << file_name << ":" << line << " in " << function_name << "]: DEBUG "
			<< int(type) - int(LOG_DEBUG_0) << ": "
			<< arg0 << arg1 << arg2 << arg3 << arg4 << arg5 << arg6 << arg7 << arg8 << arg9 << arg10 << std::flush;
		msg_first_part = ss.str();
		doFormatting(&msg_first_part, type, Log::output);
		break;
	}

	// If GLFW threads are used, write the thread's name if not "", followed by its ID.
#ifdef LOG_ENABLE_GLFW_THREADS
	{
		std::stringstream ss2;
		GLFWthread id = glfwGetThreadID();
		std::map<GLFWthread, ThreadInfo>::iterator it = thread_infos.find(id);

		if(	it != thread_infos.end() &&
			it->second.name != "")
			ss2 << "[" << it->second.name << ":" << (unsigned)id << "] ";
		else
			ss2 << "[" << (unsigned)id << "] ";

		str_temp = ss2.str();
		doFormatting(&str_temp, LOG_INFO, Log::output);
		if(Log::output == TERMINAL)
			writeTermFormattedString(p_stream, str_temp);
		else
			(*p_stream) << str_temp;
	}
#endif

	// Finally write the message
	std::string indentation = "";
	if(Log::output == TERMINAL)
	{
		for(int i=0 ; i<current_indent ; i++)
			indentation += "    ";
		(*p_stream) << indentation;
		writeTermFormattedString(p_stream, msg_first_part);
		writeTermFormattedString(p_stream, msg_second_part);

		resetTerm(p_stream);	// If the terminal was used, reset its colors before the final newline,
		(*p_stream) << std::endl;
	}
	else if(Log::output == HTML)
	{
		char str_nb_pixels[100] = "";
		sprintf(str_nb_pixels, "%d", current_indent*30);

		indentation = std::string("<span style=\"margin-left:") + std::string(str_nb_pixels) + "px;\">";

		(*p_stream) << indentation << msg_first_part << msg_second_part << " <br/>" << "</span>" << std::flush;
	}
	else if(Log::output == RTF)
	{
		for(int i=0 ; i<current_indent ; i++)
			indentation += "\\tab";
		(*p_stream) << indentation << msg_first_part << msg_second_part << "\\par" << std::flush;
	}
	else	// TXT and TERMINAL_NO_COLOR
	{
		for(int i=0 ; i<current_indent ; i++)
			indentation += "    ";
		(*p_stream) << indentation << msg_first_part << msg_second_part << std::endl;
	}

#ifdef LOG_ENABLE_GLFW_THREADS
	glfwUnlockMutex(Log::mutex);
#endif
}

#endif // LOG_H
