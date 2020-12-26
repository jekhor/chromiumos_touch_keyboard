#ifndef _LOGGING_H
#define _LOGGING_H

#include <iostream>

enum LogSeverity {
  VERBOSE,
  DEBUG,
  INFO,
  WARNING,
  ERROR,
  FATAL_WITHOUT_ABORT,
  FATAL,
};

void SetMinimumLogSeverity(enum LogSeverity severity);
std::ostream& get_log_stream(enum LogSeverity severity);

//#define LOG(severity) (get_log_stream(severity) << __FILE__ << ":" << __LINE__ << ": ")
#define LOG(severity) (get_log_stream(severity) << #severity[0] << " ")
#define PLOG(severity) LOG(severity)


#endif /* _LOGGING_H */
