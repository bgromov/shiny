/*
 * color_patternlayout.h
 *
 *  Created on: May 21, 2014
 *      Author: 0xff
 */

#ifndef COLOR_PATTERNLAYOUT_H_
#define COLOR_PATTERNLAYOUT_H_

#include <log4cxx/patternlayout.h>

using namespace std;

namespace log4cxx
{
class ColorPatternLayout : public PatternLayout
{
public:
  DECLARE_LOG4CXX_OBJECT(ColorPatternLayout)
  BEGIN_LOG4CXX_CAST_MAP()
    LOG4CXX_CAST_ENTRY(ColorPatternLayout)
    LOG4CXX_CAST_ENTRY_CHAIN(PatternLayout)
    LOG4CXX_CAST_ENTRY_CHAIN(Layout)
  END_LOG4CXX_CAST_MAP()

  void format(LogString& output, const spi::LoggingEventPtr& event,
              log4cxx::helpers::Pool& pool) const;
};

LOG4CXX_PTR_DEF(ColorPatternLayout);

} /* namespace log4cxx */

#endif /* COLOR_PATTERNLAYOUT_H_ */
