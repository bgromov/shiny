/*
 * logger.h
 *
 *  Created on: May 20, 2014
 *      Author: 0xff
 */

#ifndef LOGGER_H_
#define LOGGER_H_

#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>

//extern "C" log4cxx::LoggerPtr g_rootLogger;

// TODO: That should be fixed when logging to file
#define COLOR_NORMAL "\033[0m"
#define COLOR_RED "\033[31m"
#define COLOR_BRIGHT_RED "\033[1;31m"
#define COLOR_GREEN "\033[32m"
#define COLOR_BRIGHT_GREEN "\033[1;32m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_BRIGHT_YELLOW "\033[1;33m"
#define COLOR_WHITE "\033[37m"
#define COLOR_BRIGHT_WHITE "\033[1;37m"

#define UCS_DEBUG(msg) LOG4CXX_DEBUG(g_rootLogger, COLOR_GREEN << msg << COLOR_NORMAL)
#define UCS_INFO(msg)  LOG4CXX_INFO(g_rootLogger, COLOR_NORMAL << msg << COLOR_NORMAL)
#define UCS_WARN(msg)  LOG4CXX_WARN(g_rootLogger, COLOR_BRIGHT_YELLOW << msg << COLOR_NORMAL)
#define UCS_ERROR(msg) LOG4CXX_ERROR(g_rootLogger, COLOR_RED << msg << COLOR_NORMAL)
#define UCS_FATAL(msg) LOG4CXX_FATAL(g_rootLogger, COLOR_BRIGHT_RED << msg << COLOR_NORMAL)
//#define UCS_FATAL(...) LOG4CXX_FATAL(g_rootLogger, __VA_ARGS__)
#endif /* LOGGER_H_ */
