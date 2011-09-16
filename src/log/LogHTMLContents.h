// LogHTMLContents.h

#ifndef LOG_HTML_CONTENTS_H
#define LOG_HTML_CONTENTS_H

static const char* html_begin = STRINGIFY(
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">

<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
<head>
	<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
	<title>Log</title>
	<style type="text/css">
		div
		{
		   font-family: monospace;
		}

		.info
		{
		}

		.success
		{
			color: green;
			font-weight: bold;
		}

		.failed
		{
			color: red;
			font-weight: bold;
		}

		.warn
		{
			color: white;
			background-color: orange;
			font-weight: bold;
		}

		.error
		{
			color: white;
			background-color: red;
			font-weight: bold;
		}

		.debug0
		{
			color: white;
			background-color: green;
		}

		.debug1
		{
			color: white;
			background-color: blue;
		}

		.debug2
		{
			color: white;
			background-color: purple;
		}

		.debug3
		{
			color: white;
			background-color: teal;
		}

		.debug4
		{
			color: white;
			background-color: gray;
		}

		.debug5
		{
			color: yellow;
			background-color: green;
		}

		.debug6
		{
			color: yellow;
			background-color: blue;
		}

		.debug7
		{
			color: yellow;
			background-color: purple;
		}

		.debug8
		{
			color: yellow;
			background-color: teal;
		}

		.debug9
		{
			color: yellow;
			background-color: gray;
		}
	</style>
</head>
<body>
	<div>
);

static const char* html_end = STRINGIFY(
	</div>
</body>
</html>
);

#endif // LOG_HTML_CONTENTS_H
