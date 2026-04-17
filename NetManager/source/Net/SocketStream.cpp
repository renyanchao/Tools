#include "SocketStream.h"

int32_t GetSocketCanRead(socket_t fd)
{
	unsigned long bytes = 0;
#ifdef _WIN32
	ioctlsocket(fd, FIONREAD, &bytes);
#else
	ioctl(m_fd, FIONREAD, &bytes)
#endif
		return bytes;
}

bool Socket_IStream::Fill(socket_t* pSocket)
{
	if (pSocket == nullptr)return false;
	socket_t fd = *pSocket;
	size_t nCanReadSize = GetSocketCanRead(fd);
	if (nCanReadSize <= 0)return false;


	if (GetFreeSize() < nCanReadSize)
	{
		Resize(nCanReadSize);
	}
	size_t this_write_size = min(nCanReadSize, GetFreeSize());
	if (this_write_size <= 0)return false;
	// 分两段写入（可能绕回）
	size_t total = m_nSize + 1;
	size_t first_chunk = min(this_write_size, total - m_WritePos);

	size_t nTotalReadSize = 0;
	if (first_chunk > 0)
	{
		nTotalReadSize += recv(fd, &m_pBuffer[m_WritePos], first_chunk, 0);
		m_WritePos = (m_WritePos + first_chunk) % total;
	}
	size_t second_chunk = this_write_size - first_chunk;
	if (second_chunk > 0)
	{
		nTotalReadSize += recv(fd, &m_pBuffer[m_WritePos], second_chunk, 0);
		m_WritePos = (m_WritePos + second_chunk) % total;
	}
	return nTotalReadSize > 0;
}

bool Socket_OStream::Flush(socket_t* pSocket)
{
	if (GetUsedSize() == 0)return true;
	if (pSocket == nullptr)return false;
	socket_t fd = *pSocket;

	size_t total = m_nSize + 1;
	size_t readable = GetUsedSize();
	size_t read_len = readable;
	size_t nTotalSendSize = 0;
	size_t first_chunk = min(read_len, total - m_ReadPos);
	//std::memcpy(buffer, &m_pBuffer[m_ReadPos], first_chunk);
	if (first_chunk > 0)
	{
		nTotalSendSize += send(fd, &m_pBuffer[m_ReadPos], first_chunk, 0);
		m_ReadPos = (m_ReadPos + first_chunk) % total;
	}
	size_t second_chunk = read_len - first_chunk;
	if (second_chunk > 0) {
		/*std::memcpy(static_cast<uint8_t*>(buffer) + first_chunk,
			&m_pBuffer[m_ReadPos], second_chunk);*/
		nTotalSendSize += send(fd, &m_pBuffer[m_ReadPos], second_chunk, 0);
		m_ReadPos = (m_ReadPos + second_chunk) % total;
	}
	return nTotalSendSize > 0;
}