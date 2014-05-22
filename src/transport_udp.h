/*
 * transport_udp.h
 *
 *  Created on: May 20, 2014
 *      Author: 0xff
 */

#ifndef TRANSPORT_UDP_H_
#define TRANSPORT_UDP_H_

#include "transport.h"
#include "logger.h"

namespace UCS
{

namespace Transport
{
enum
{
  UDPMaxDatagramSize = 1500,
  InvalidSocket = -1,
};

class UDP: public AbstractTransport, public Module
{
  int sock_;
  size_t max_datagram_size_;
  size_t last_seq_no_;

  bool closed_;

public:
  UDP(size_t max_datagram_size, const log4cxx::LoggerPtr& logger);
  UDP(size_t max_datagram_size): UDP(max_datagram_size, log4cxx::Logger::getRootLogger()) {}
  ~UDP();

  //-- Transport interface
  string getType() const {return "UDP";}

  ssize_t read(uint8_t* buf, size_t size);
  ssize_t write(uint8_t* buf, size_t size);

  void close();

  //-- Module interface
  string getModuleName() const {return "udp";}

  //-- UDP
  bool createIncoming(int port);
  bool createOutgoing(const string& host, int port);

  bool connect(const string& host, int port, int conn_id);
  size_t getMaxDatagramSize() const {return max_datagram_size_;}
};

using UDPPtr = shared_ptr<UDP>;

} // namespace Transport

} /* namespace UCS */

#endif /* TRANSPORT_UDP_H_ */
