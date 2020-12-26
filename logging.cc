#include <iostream>

#include "logging.h"

class NullBuffer : public std::streambuf
{
public:
  int overflow(int c) { return c; }
};

NullBuffer null_buffer;
std::ostream null_stream(&null_buffer);

enum LogSeverity min_severity = INFO;

std::ostream& get_log_stream(enum LogSeverity severity)
{
	if (severity >= min_severity)
		return std::cout;
	else
		return null_stream;
}

void SetMinimumLogSeverity(enum LogSeverity severity)
{
	min_severity = severity;
}
