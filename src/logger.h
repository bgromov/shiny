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
#include <log4cxx/patternlayout.h>

extern "C" log4cxx::LoggerPtr g_rootLogger;

#define UCS_DEBUG(msg) LOG4CXX_DEBUG(g_rootLogger, msg)
#define UCS_INFO(msg)  LOG4CXX_INFO(g_rootLogger, msg)
#define UCS_WARN(msg)  LOG4CXX_WARN(g_rootLogger, msg)
#define UCS_ERROR(msg) LOG4CXX_ERROR(g_rootLogger, msg)
#define UCS_FATAL(msg) LOG4CXX_FATAL(g_rootLogger, msg)
#define UCS_TRACE(msg) LOG4CXX_TRACE(g_rootLogger, msg)
//#define UCS_FATAL(...) LOG4CXX_FATAL(g_rootLogger, __VA_ARGS__)


#endif /* LOGGER_H_ */
