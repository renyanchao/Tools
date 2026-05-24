#pragma once

#include <cstddef>
#include <cstdint>

#ifdef _WIN32
using socket_t = uintptr_t;
#else
using socket_t = int;
#endif

constexpr socket_t INVALID_SOCKET_VAL = static_cast<socket_t>(-1);
constexpr int SOCKET_ERROR_VAL = -1;

int GET_LAST_ERROR();
void close_socket(socket_t s);

struct SocketStream
{
public:
    char*  m_pBuffer = nullptr;
    size_t m_nSize = 0;
    size_t m_ReadPos = 0;
    size_t m_WritePos = 0;
private:
    size_t GetBuffSize()const { return m_nSize + 1; }
public:
    SocketStream(int32_t size);
    ~SocketStream();

    bool IsEmpty() const;
    bool IsFull() const;
    size_t GetFreeSize() const;
    size_t GetUsedSize() const;

    size_t Write(const void* data, size_t len);
    size_t Read(void* buffer, size_t len);
    bool Resize(int32_t nAddSize);

    bool Peek(void* buffer, size_t len);
    bool Fill(socket_t* fd);
    bool Flush(socket_t* fd);
};
