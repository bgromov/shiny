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

//#define UCS_DEBUG(msg) LOG4CXX_DEBUG(g_rootLogger, msg)
//#define UCS_INFO(msg)  LOG4CXX_INFO(g_rootLogger, msg)
//#define UCS_WARN(msg)  LOG4CXX_WARN(g_rootLogger, msg)
//#define UCS_ERROR(msg) LOG4CXX_ERROR(g_rootLogger, msg)
//#define UCS_FATAL(msg) LOG4CXX_FATAL(g_rootLogger, msg)
//#define UCS_TRACE(msg) LOG4CXX_TRACE(g_rootLogger, msg)

#define UCS_DEBUG(msg) LOG4CXX_DEBUG(module_logger, msg)
#define UCS_INFO(msg)  LOG4CXX_INFO(module_logger, msg)
#define UCS_WARN(msg)  LOG4CXX_WARN(module_logger, msg)
#define UCS_ERROR(msg) LOG4CXX_ERROR(module_logger, msg)
#define UCS_FATAL(msg) LOG4CXX_FATAL(module_logger, msg)
#define UCS_TRACE(msg) LOG4CXX_TRACE(module_logger, msg)
//#define UCS_FATAL(...) LOG4CXX_FATAL(g_rootLogger, __VA_ARGS__)

namespace UCS
{
using namespace std;

class Module
{
protected:
  log4cxx::LoggerPtr module_logger;
  string module_name_;
public:
  Module(const string& module_name): Module(log4cxx::Logger::getRootLogger(), module_name) {}
  Module(const log4cxx::LoggerPtr& root_logger, const string& module_name)
  {
    string logger_name;

    module_name_ = module_name;

    if(root_logger == log4cxx::Logger::getRootLogger())
      logger_name = module_name_;
    else
      logger_name = root_logger->getName() + "." + module_name_;

    module_logger = root_logger->getLogger(logger_name);
  }

  virtual ~Module(){};

  virtual string getModuleName() const = 0;
};

} /* namespace UCS */

#endif /* LOGGER_H_ */
