/*
 * transport.h
 *
 *  Created on: Apr 28, 2014
 *      Author: 0xff
 */

#ifndef TRANSPORT_H_
#define TRANSPORT_H_

#include <unistd.h>
#include <stdarg.h>
#include <sys/socket.h>

#include "eventhandler.h"

namespace UCS
{

using namespace std;

void debug(const char *format, ...)
{
  va_list argptr = NULL;
  va_start(argptr, format);

  vfprintf(stderr, format, argptr);

  va_end(argptr);
}

class Transport;
using TransportPtr = typename shared_ptr<Transport>;

class Transport: public UCSEventHandler
{
  UCSListenerList<> onRead;
  UCSListenerList<> onWrite;

public:
  virtual ~Transport() {}

  virtual ssize_t read(uint8_t* buf, size_t size) = 0;
  virtual ssize_t write(uint8_t* buf, size_t size) = 0;

  virtual void close() = 0;
  virtual string getType() = 0;
};

} // namespace UCS

#endif /* TRANSPORT_H_ */
