#pragma once

#include<iostream>
#include<memory>
#include<cstring>
#include<math.h>

struct Socket_IOStream
{
public:
    char*   m_pBuffer = nullptr;
    size_t  m_nSize = 0;
    size_t  m_ReadPos = 0;
    size_t  m_WritePos = 0;

public:
    Socket_IOStream(int32_t size){m_nSize = size; m_pBuffer = new char[size];}
    bool IsEmpty()const{ return m_ReadPos == m_WritePos; }
    bool IsFull()const{ return (m_WritePos + 1) % (m_nSize + 1) == m_ReadPos; }
    size_t GetFreeSize()const{ return m_nSize - GetUsedSize(); }
    size_t GetUsedSize()const
    {
        if (m_WritePos >= m_ReadPos)
            return m_WritePos - m_ReadPos;
        return m_WritePos + (m_nSize + 1) - m_ReadPos;
    }

    size_t Write(const void* data, size_t len)
    {
        if(GetFreeSize() < len)
        {
            Resize(len);
        }
        size_t this_write_size = std::min(len, GetFreeSize());
        if(this_write_size <= 0)return 0;
        // 分两段写入（可能绕回）
        size_t total = m_nSize + 1;
        size_t first_chunk = std::min(this_write_size, total - m_WritePos);
        memcpy(&m_pBuffer[m_WritePos], data, first_chunk);
        m_WritePos = (m_WritePos + first_chunk) % total;

        size_t second_chunk = this_write_size - first_chunk;
        if (second_chunk > 0) {
            std::memcpy(&m_pBuffer[m_WritePos],
                        static_cast<const uint8_t*>(data) + first_chunk,
                        second_chunk);
            m_WritePos = (m_WritePos + second_chunk) % total;
        }

        return this_write_size;
    }
    // 读取最多 len 字节到 buffer，返回实际读取的字节数
    size_t Read(void* buffer, size_t len) 
    {
        if (len == 0) return 0;
    
        size_t total = m_nSize + 1;
        size_t readable = GetUsedSize();
        size_t read_len = std::min(len, readable);

        size_t first_chunk = std::min(read_len, total - m_ReadPos);
        std::memcpy(buffer, &m_pBuffer[m_ReadPos], first_chunk);
        m_ReadPos = (m_ReadPos + first_chunk) % total;

        size_t second_chunk = read_len - first_chunk;
        if (second_chunk > 0) {
            std::memcpy(static_cast<uint8_t*>(buffer) + first_chunk,
                        &m_pBuffer[m_ReadPos], second_chunk);
            m_ReadPos = (m_ReadPos + second_chunk) % total;
        }
        return read_len;
    }
    bool Resize(int32_t nAddSize)
    {
        int32_t nNewSize = std::max(m_nSize + nAddSize, m_nSize>>1);
        char* pNewBuffer = new char[m_nSize];
        if (pNewBuffer == nullptr)
        {
            return false;
        }
        int32_t nUsedSize = GetUsedSize();
        int32_t nCopySize = Read(pNewBuffer, GetUsedSize());
        if(nCopySize != nUsedSize)
        {
            return false;
        }
        delete []m_pBuffer;
        m_pBuffer = pNewBuffer;
        m_ReadPos = 0;
        m_WritePos = nUsedSize;
        m_nSize = nNewSize;
    }

    bool Peek(void* buffer, size_t len)
    {
        if (len == 0 || GetUsedSize() < len) return false;
        size_t total = m_nSize + 1;
        size_t readable = GetUsedSize();
        size_t read_len = std::min(len, readable);
        size_t __read_pos = m_ReadPos;

        size_t first_chunk = std::min(read_len, total - __read_pos);
        std::memcpy(buffer, &m_pBuffer[__read_pos], first_chunk);
        __read_pos = (__read_pos + first_chunk) % total;

        size_t second_chunk = read_len - first_chunk;
        if (second_chunk > 0) {
            std::memcpy(static_cast<uint8_t*>(buffer) + first_chunk,
                        &m_pBuffer[__read_pos], second_chunk);
            __read_pos = (__read_pos + second_chunk) % total;
        }
        return true;
    }

};

#ifdef _WIN32
#define _WIN32_WINNT 0x0601
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
typedef SOCKET socket_t;
#define INVALID_SOCKET_VAL INVALID_SOCKET
#define SOCKET_ERROR_VAL   SOCKET_ERROR
#define close_socket(s)    closesocket(s)
#define GET_LAST_ERROR()   WSAGetLastError()
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <errno.h>
#include <fcntl.h>
typedef int socket_t;
#define INVALID_SOCKET_VAL (-1)
#define SOCKET_ERROR_VAL   (-1)
#define close_socket(s)    close(s)
#define GET_LAST_ERROR()   errno
#endif

struct Socket_IStream : Socket_IOStream
{
    Socket_IStream(int32_t size) :Socket_IOStream(size){}
    /* data */
    bool Fill(socket_t* fd);
    
};

struct Socket_OStream : Socket_IOStream
{
    /* data */
    Socket_OStream(int32_t size) :Socket_IOStream(size) {}
    bool Flush(socket_t* fd);
};

