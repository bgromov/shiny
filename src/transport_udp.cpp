/*
 * transport_udp.cpp
 *
 *  Created on: May 20, 2014
 *      Author: 0xff
 */

#include "transport_udp.h"
#include "logger.h"

namespace UCS
{
namespace Transport
{

UDP::UDP():
  max_datagram_size_(UDPMaxDatagramSize),
  sock_(InvalidSocket),
  last_seq_no_(0),
  closed_(true)
{
  // TODO Auto-generated constructor stub
}

UDP::~UDP()
{
  // TODO Auto-generated destructor stub
}

ssize_t UDP::read(uint8_t* buf, size_t size)
{
  if(closed_)
  {
    UCS_DEBUG("Attempt to read on a closed socket [" << sock_ << "]");
    return -1;
  }

  return -1;
}

ssize_t UDP::write(uint8_t* buf, size_t size)
{
  if(closed_)
  {
    UCS_DEBUG("Attempt to write on a closed socket [" << sock_ << "]");
    return -1;
  }

  return -1;
}

void UDP::close()
{
  if(!closed_)
  {
    if(sock_ != InvalidSocket)
    {
      ::close(sock_);
      closed_ = true;
    }
    else
    {
      UCS_DEBUG("Something went wrong. Attempted to close invalid socket.");
      closed_ = true;
    }
  }
}

bool UDP::connect(const string& host, int port, int conn_id)
{
  return false;
}

} /* namespace Transport */
} /* namespace UCS */
