/*
 * color_patternlayout.cpp
 *
 *  Created on: May 21, 2014
 *      Author: 0xff
 */

#include "color_patternlayout.h"

#define COLOR_NORMAL "\033[0m"
#define COLOR_RED "\033[31m"
#define COLOR_BRIGHT_RED "\033[1;31m"
#define COLOR_GREEN "\033[32m"
#define COLOR_BRIGHT_GREEN "\033[1;32m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_BRIGHT_YELLOW "\033[1;33m"
#define COLOR_WHITE "\033[37m"
#define COLOR_BRIGHT_WHITE "\033[1;37m"

#define MAKE_NO_COLOR(str) (str)
#define MAKE_RED(str) (COLOR_RED + str + COLOR_NORMAL)
#define MAKE_BRIGHT_RED(str) (COLOR_BRIGHT_RED + str + COLOR_NORMAL)
#define MAKE_GREEN(str) (COLOR_GREEN + str + COLOR_NORMAL)
#define MAKE_BRIGHT_GREEN(str) (COLOR_BRIGHT_GREEN + str + COLOR_NORMAL)
#define MAKE_YELLOW(str) (COLOR_YELLOW + str + COLOR_NORMAL)
#define MAKE_BRIGHT_YELLOW(str) (COLOR_BRIGHT_YELLOW+ str + COLOR_NORMAL)
#define MAKE_WHITE(str) (COLOR_WHITE + str + COLOR_NORMAL)
#define MAKE_BRIGHT_WHITE(str) (COLOR_BRIGHT_WHITE+ str + COLOR_NORMAL)

using namespace log4cxx;
using namespace log4cxx::helpers;

IMPLEMENT_LOG4CXX_OBJECT(ColorPatternLayout)

void ColorPatternLayout::format(LogString& output, const spi::LoggingEventPtr& event,
                                log4cxx::helpers::Pool& pool) const
{
  LogString temp_out, color;

  PatternLayout::format(temp_out, event, pool);

  switch(event->getLevel()->toInt())
  {
    case Level::FATAL_INT:
      color = COLOR_BRIGHT_RED;
      break;
    case Level::ERROR_INT:
      color = COLOR_RED;
      break;
    case Level::WARN_INT:
      color = COLOR_BRIGHT_YELLOW;
      break;
    case Level::INFO_INT:
      // maybe no color?
      color = COLOR_BRIGHT_WHITE;
      break;
    case Level::DEBUG_INT:
      color = COLOR_GREEN;
      break;
    case Level::TRACE_INT:
    default:
      // no color
      break;
  }

  output = color + temp_out + COLOR_NORMAL;
}
