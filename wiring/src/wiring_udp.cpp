/**
 ******************************************************************************
  Copyright (c) 2013-2014 IntoRobot Team.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation, either
  version 3 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, see <http://www.gnu.org/licenses/>.
  ******************************************************************************
*/

#include "intorobot_config.h"

#ifndef configNO_NETWORK

#include "wiring_udp.h"
#include "inet_hal.h"
#include "intorobot_macros.h"
#include "wiring_network.h"
#include "wlan_hal.h"
#include "wiring_constants.h"

/*debug switch*/
//#define WIRING_UDP_DEBUG

#ifdef WIRING_UDP_DEBUG
#define WUDP_DEBUG(...)    do {DEBUG(__VA_ARGS__);}while(0)
#define WUDP_DEBUG_D(...)  do {DEBUG_D(__VA_ARGS__);}while(0)
#define WUDP_DEBUG_DUMP    DEBUG_DUMP
#else
#define WUDP_DEBUG(...)
#define WUDP_DEBUG_D(...)
#define WUDP_DEBUG_DUMP
#endif

using namespace intorobot;

static bool inline isOpen(sock_handle_t sd)
{
   return sd != socket_handle_invalid();
}

UDP::UDP() : _sock(socket_handle_invalid()), _offset(0), _total(0), _buffer(0), _buffer_size(512)
{
}

bool UDP::setBuffer(size_t buf_size, uint8_t* buffer)
{
    releaseBuffer();

    _buffer = buffer;
    _buffer_size = 0;
    if (!_buffer && buf_size) {         // requested allocation
        _buffer = new uint8_t[buf_size];
        _buffer_allocated = true;
    }
    if (_buffer) {
        _buffer_size = buf_size;
    }
    return _buffer_size;
}

void UDP::releaseBuffer()
{
    if (_buffer_allocated && _buffer) {
        delete _buffer;
    }
    _buffer = NULL;
    _buffer_allocated = false;
    _buffer_size = 0;
    flush();
}

uint8_t UDP::begin(uint16_t port, network_interface_t nif)
{
    return begin(IPAddress(IPADDR_ANY), 8888, port, nif);
}

uint8_t UDP::begin(IPAddress remoteIP, uint16_t remotePort, uint16_t port, network_interface_t nif)
{
    bool bound = 0;
    if(Network.from(nif).ready())
    {
        _sock = socket_create(AF_INET, SOCK_DGRAM, IPPROTO_UDP, port, nif);
        if (socket_handle_valid(_sock))
        {
            sockaddr_t tSocketAddr;
            int connected = 0;

            WUDP_DEBUG("udp begin success! create socket %d\r\n",_sock);
            flush();
            tSocketAddr.sa_family = AF_INET;
            tSocketAddr.sa_data[0] = (remotePort & 0xFF00) >> 8;
            tSocketAddr.sa_data[1] = (remotePort & 0x00FF);
            tSocketAddr.sa_data[2] = remoteIP[0];        // Todo IPv6
            tSocketAddr.sa_data[3] = remoteIP[1];
            tSocketAddr.sa_data[4] = remoteIP[2];
            tSocketAddr.sa_data[5] = remoteIP[3];

            uint32_t ot = HAL_NET_SetNetWatchDog(S2M(MAX_SEC_WAIT_CONNECT));
            connected = (socket_connect(_sock, &tSocketAddr, sizeof(tSocketAddr)) == 0 ? 1 : 0);
            HAL_NET_SetNetWatchDog(ot);
            if(connected)
            {
                WUDP_DEBUG("upd connect success! create socket %d\r\n", _sock);
            }
            else
            {
                WUDP_DEBUG("upd connect failed!\r\n");
                stop();
            }
            _remoteIP = remoteIP;
            _remotePort = remotePort;
            _port = port;
            _nif = nif;
            bound = true;
        }
        else {
            WUDP_DEBUG("udp begin failed!\r\n");
            stop();
            bound = false;
        }
    }
    return bound;
}

int UDP::available()
{
    return _total - _offset;
}

void UDP::stop()
{
    WUDP_DEBUG("udp stop: close socket %d\r\n", _sock);
    if (isOpen(_sock))
    {
        socket_close(_sock);
    }
    _sock = socket_handle_invalid();
    flush();
}

int UDP::beginPacket()
{
    return beginPacket(_remoteIP, _remotePort);
}

int UDP::beginPacket(const char *remoteHost, uint16_t remotePort)
{
    if(Network.from(_nif).ready())
    {
        HAL_IPAddress ip_addr;

        if(inet_gethostbyname((char*)remoteHost, strlen(remoteHost), &ip_addr, _nif, NULL) == 0)
        {
            IPAddress remote_addr(ip_addr);
            return beginPacket(remote_addr, remotePort);
        }
    }
    return 0;
}

int UDP::beginPacket(IPAddress remoteIP, uint16_t remotePort)
{
    // default behavior previously was to use a 512 byte buffer, so instantiate that if not already done
    if (!_buffer && _buffer_size) {
        setBuffer(_buffer_size);
    }

    _remoteIP = remoteIP;
    _remotePort = remotePort;
    flush();
    return _buffer_size;
}

int UDP::endPacket()
{
    int result = sendPacket(_buffer, _offset, _remoteIP, _remotePort);
    flush();
    return result;
}

int UDP::sendPacket(const uint8_t* buffer, size_t buffer_size, IPAddress remoteIP, uint16_t port)
{
    sockaddr_t remoteSockAddr;
    remoteSockAddr.sa_family = AF_INET;

    remoteSockAddr.sa_data[0] = (port & 0xFF00) >> 8;
    remoteSockAddr.sa_data[1] = (port & 0x00FF);

    remoteSockAddr.sa_data[2] = remoteIP[0];
    remoteSockAddr.sa_data[3] = remoteIP[1];
    remoteSockAddr.sa_data[4] = remoteIP[2];
    remoteSockAddr.sa_data[5] = remoteIP[3];

    int rv = socket_sendto(_sock, buffer, buffer_size, 0, &remoteSockAddr, sizeof(remoteSockAddr));
    WUDP_DEBUG("udp sendPacket(buffer=%lx, size=%d)=%d\r\n",buffer, buffer_size, rv);
    return rv;
}

size_t UDP::write(uint8_t byte)
{
    return write(&byte, 1);
}

size_t UDP::write(const uint8_t *buffer, size_t size)
{
    size_t available = _buffer ? _buffer_size - _offset : 0;
    if (size>available)
        size = available;
    memcpy(_buffer+_offset, buffer, size);
    _offset += size;
    return size;
}

int UDP::parsePacket()
{
    if (!_buffer && _buffer_size) {
        setBuffer(_buffer_size);
    }

    flush();         // start a new read - discard the old data
    if (_buffer && _buffer_size) {
        int result = receivePacket(_buffer, _buffer_size);
        if (result>0) {
            _total = result;
        }
    };
    return available();
}

int UDP::receivePacket(uint8_t* buffer, size_t size)
{
    int ret = -1;
    if(Network.from(_nif).ready() && isOpen(_sock) && buffer)
    {
        sockaddr_t remoteSockAddr;
        socklen_t remoteSockAddrLen = sizeof(remoteSockAddr);

        ret = socket_receivefrom(_sock, buffer, size, 0, &remoteSockAddr, &remoteSockAddrLen);
        if (ret > 0)
        {
            _remotePort = remoteSockAddr.sa_data[0] << 8 | remoteSockAddr.sa_data[1];
            _remoteIP = &remoteSockAddr.sa_data[2];
        }
    }
    return ret;
}

int UDP::read()
{
  return available() ? _buffer[_offset++] : -1;
}

int UDP::read(unsigned char* buffer, size_t len)
{
    int read = -1;
    if (available())
    {
        read = min(int(len), available());
        memcpy(buffer, &_buffer[_offset], read);
        _offset += read;
    }
    return read;
}

int UDP::peek()
{
    return available() ? _buffer[_offset] : -1;
}

void UDP::flush()
{
  _offset = 0;
  _total = 0;
}

size_t UDP::printTo(Print& p) const
{
    // can't use available() since this is a `const` method, and available is part of the Stream interface, and is non-const.
    int size = _total - _offset;
    return p.write(_buffer+_offset, size);
}

int UDP::joinMulticast(const IPAddress& ip)
{
    if (_sock == socket_handle_invalid())
        return -1;
    HAL_IPAddress address = ip.raw();
    return socket_join_multicast(&address, _nif, 0);
}

int UDP::leaveMulticast(const IPAddress& ip)
{
    if (_sock == socket_handle_invalid())
        return -1;
    HAL_IPAddress address = ip.raw();
    return socket_leave_multicast(&address, _nif, 0);
}

#endif
