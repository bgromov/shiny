/*
 * transport.h
 *
 *  Created on: Apr 28, 2014
 *      Author: 0xff
 */

#ifndef TRANSPORT_H_
#define TRANSPORT_H_

#include <sys/socket.h>

#include "eventhandler.h"

namespace UCS
{

using namespace std;

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

namespace Transport
{
enum
{
  UDPMaxDatagramSize = 1500,
  InvalidSocket = -1,
};

class UDP;
using UDPPtr = typename shared_ptr<UDP>;

class UDP: public Transport
{
  size_t max_datagram_size_;
  int sock_;

  size_t last_seq_no_;

public:
  UDP(): max_datagram_size_(UDPMaxDatagramSize), sock_(InvalidSocket) {}

  bool connect(const string& host, int port, int conn_id);

};

} // namespace Transport

} // namespace UCS

#endif /* TRANSPORT_H_ */
