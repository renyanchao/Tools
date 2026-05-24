#include "SocketStream.h"

#include <algorithm>
#include <cstring>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#define _WIN32_WINNT 0x0601
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

SocketStream::SocketStream(int32_t size)
{
    m_nSize = size;
    m_pBuffer = new char[GetBuffSize()];
}

SocketStream::~SocketStream()
{
    delete[] m_pBuffer;
}

bool SocketStream::IsEmpty() const
{
    return m_ReadPos == m_WritePos;
}

bool SocketStream::IsFull() const
{
    return (m_WritePos + 1) % GetBuffSize() == m_ReadPos;
}

size_t SocketStream::GetFreeSize() const
{
    return m_nSize - GetUsedSize();
}

size_t SocketStream::GetUsedSize() const
{
    if (m_WritePos >= m_ReadPos)
        return m_WritePos - m_ReadPos;
    return m_WritePos + GetBuffSize() - m_ReadPos;
}

int GET_LAST_ERROR()
{
#ifdef _WIN32
    return WSAGetLastError();
#else
    return errno;
#endif
}

void close_socket(socket_t s)
{
#ifdef _WIN32
    closesocket(static_cast<SOCKET>(s));
#else
    close(s);
#endif
}

static int32_t GetSocketCanRead(socket_t fd)
{
#ifdef _WIN32
    unsigned long bytes = 0;
    ioctlsocket(static_cast<SOCKET>(fd), FIONREAD, &bytes);
    return static_cast<int32_t>(bytes);
#else
    int bytes = 0;
    ioctl(fd, FIONREAD, &bytes);
    return bytes;
#endif
}

static bool IsWouldBlockError(int error)
{
#ifdef _WIN32
    return error == WSAEWOULDBLOCK;
#else
    return error == EAGAIN || error == EWOULDBLOCK;
#endif
}

static bool IsSocketInterruptedError(int error)
{
#ifdef _WIN32
    return error == WSAEINTR;
#else
    return error == EINTR;
#endif
}

size_t SocketStream::Write(const void* data, size_t len)
{
    if (GetFreeSize() < len)
    {
        Resize(static_cast<int32_t>(len));
    }
    size_t this_write_size = std::min(len, GetFreeSize());
    if (this_write_size == 0) return 0;

    size_t first_chunk = std::min(this_write_size, GetBuffSize() - m_WritePos);
    std::memcpy(&m_pBuffer[m_WritePos], data, first_chunk);
    m_WritePos = (m_WritePos + first_chunk) % GetBuffSize();

    size_t second_chunk = this_write_size - first_chunk;
    if (second_chunk > 0) {
        std::memcpy(&m_pBuffer[m_WritePos],
                    static_cast<const std::uint8_t*>(data) + first_chunk,
                    second_chunk);
        m_WritePos = (m_WritePos + second_chunk) % GetBuffSize();
    }

    return this_write_size;
}

size_t SocketStream::Read(void* buffer, size_t len)
{
    if (len == 0) return 0;

    size_t readable = GetUsedSize();
    size_t read_len = std::min(len, readable);

    size_t first_chunk = std::min(read_len, GetBuffSize() - m_ReadPos);
    std::memcpy(buffer, &m_pBuffer[m_ReadPos], first_chunk);
    m_ReadPos = (m_ReadPos + first_chunk) % GetBuffSize();

    size_t second_chunk = read_len - first_chunk;
    if (second_chunk > 0) {
        std::memcpy(static_cast<std::uint8_t*>(buffer) + first_chunk,
                    &m_pBuffer[m_ReadPos], second_chunk);
        m_ReadPos = (m_ReadPos + second_chunk) % GetBuffSize();
    }
    return read_len;
}

bool SocketStream::Resize(int32_t nAddSize)
{
    int32_t nNewSize = std::max(static_cast<int32_t>(GetBuffSize() + nAddSize), static_cast<int32_t>(GetBuffSize() << 1));
    char* pNewBuffer = new char[nNewSize];
    if (pNewBuffer == nullptr)
    {
        return false;
    }

    const size_t nUsedSize = GetUsedSize();
    const size_t nCopySize = Read(pNewBuffer, nUsedSize);
    if (nCopySize != nUsedSize)
    {
        delete[] pNewBuffer;
        return false;
    }

    delete[] m_pBuffer;
    m_pBuffer = pNewBuffer;
    m_ReadPos = 0;
    m_WritePos = nUsedSize;
    m_nSize = static_cast<size_t>(nNewSize) - 1;
    return true;
}

bool SocketStream::Peek(void* buffer, size_t len)
{
    if (len == 0 || GetUsedSize() < len) return false;

    size_t readable = GetUsedSize();
    size_t read_len = std::min(len, readable);
    size_t read_pos = m_ReadPos;

    size_t first_chunk = std::min(read_len, GetBuffSize() - read_pos);
    std::memcpy(buffer, &m_pBuffer[read_pos], first_chunk);
    read_pos = (read_pos + first_chunk) % GetBuffSize();

    size_t second_chunk = read_len - first_chunk;
    if (second_chunk > 0) {
        std::memcpy(static_cast<std::uint8_t*>(buffer) + first_chunk,
                    &m_pBuffer[read_pos], second_chunk);
    }
    return true;
}

bool SocketStream::Fill(socket_t* pSocket)
{
    if (pSocket == nullptr) return false;

    socket_t fd = *pSocket;
    int32_t nCanReadSize = GetSocketCanRead(fd);
    if (nCanReadSize <= 0) return false;

    if (GetFreeSize() < static_cast<size_t>(nCanReadSize))
    {
        if (Resize(nCanReadSize) == false)
        {
            return false;
        }
    }

    size_t this_write_size = std::min(static_cast<size_t>(nCanReadSize), GetFreeSize());
    if (this_write_size == 0) return false;

    size_t first_chunk = std::min(this_write_size, GetBuffSize() - m_WritePos);

    size_t nTotalReadSize = 0;
    if (first_chunk > 0)
    {
        int nReadSize = recv(fd, &m_pBuffer[m_WritePos], static_cast<int>(first_chunk), 0);
        if (nReadSize > 0)
        {
            nTotalReadSize += static_cast<size_t>(nReadSize);
            m_WritePos = (m_WritePos + static_cast<size_t>(nReadSize)) % GetBuffSize();
        }
        else if (nReadSize == 0)
        {
            return false;
        }
        else
        {
            int error = GET_LAST_ERROR();
            return IsWouldBlockError(error) || IsSocketInterruptedError(error);
        }
    }

    size_t second_chunk = this_write_size - nTotalReadSize;
    if (second_chunk > 0)
    {
        int nReadSize = recv(fd, &m_pBuffer[m_WritePos], static_cast<int>(second_chunk), 0);
        if (nReadSize > 0)
        {
            nTotalReadSize += static_cast<size_t>(nReadSize);
            m_WritePos = (m_WritePos + static_cast<size_t>(nReadSize)) % GetBuffSize();
        }
        else if (nReadSize == 0)
        {
            return false;
        }
        else
        {
            int error = GET_LAST_ERROR();
            return nTotalReadSize > 0 || IsWouldBlockError(error) || IsSocketInterruptedError(error);
        }
    }
    return nTotalReadSize > 0;
}

bool SocketStream::Flush(socket_t* pSocket)
{
    if (GetUsedSize() == 0) return true;
    if (pSocket == nullptr) return false;

    socket_t fd = *pSocket;

    size_t read_len = GetUsedSize();
    size_t nTotalSendSize = 0;
    size_t first_chunk = std::min(read_len, GetBuffSize() - m_ReadPos);
    if (first_chunk > 0)
    {
        int nSendSize = send(fd, &m_pBuffer[m_ReadPos], static_cast<int>(first_chunk), 0);
        if (nSendSize > 0)
        {
            nTotalSendSize += static_cast<size_t>(nSendSize);
            m_ReadPos = (m_ReadPos + static_cast<size_t>(nSendSize)) % GetBuffSize();
        }
        else if (nSendSize == 0)
        {
            return true;
        }
        else
        {
            int error = GET_LAST_ERROR();
            return IsWouldBlockError(error) || IsSocketInterruptedError(error);
        }
    }

    size_t second_chunk = read_len - nTotalSendSize;
    if (second_chunk > 0) {
        int nSendSize = send(fd, &m_pBuffer[m_ReadPos], static_cast<int>(second_chunk), 0);
        if (nSendSize > 0)
        {
            nTotalSendSize += static_cast<size_t>(nSendSize);
            m_ReadPos = (m_ReadPos + static_cast<size_t>(nSendSize)) % GetBuffSize();
        }
        else if (nSendSize == 0)
        {
            return true;
        }
        else
        {
            int error = GET_LAST_ERROR();
            return nTotalSendSize > 0 || IsWouldBlockError(error) || IsSocketInterruptedError(error);
        }
    }
    return true;
}
