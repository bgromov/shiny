/*
 * transport_udp.h
 *
 *  Created on: May 20, 2014
 *      Author: 0xff
 */

#ifndef TRANSPORT_UDP_H_
#define TRANSPORT_UDP_H_

#include "transport.h"

namespace UCS
{

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

  bool closed_;

public:
  UDP();

  //-- Transport interface
  string getType() {return "UDP";}

  ssize_t read(uint8_t* buf, size_t size);
  ssize_t write(uint8_t* buf, size_t size);

  void close();

  //-- UDP
  bool connect(const string& host, int port, int conn_id);

};

} // namespace Transport

} /* namespace UCS */

#endif /* TRANSPORT_UDP_H_ */
