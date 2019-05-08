// Copyright (c) 2016 CloudMakers, s. r. o.
// Copyright (c) 2016 Rumen G.Bogdanovski
// All rights reserved.
//
// You can use this software under the terms of 'INDIGO Astronomy
// open-source license' (see LICENSE.md).
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHORS 'AS IS' AND ANY EXPRESS
// OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
// GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// version history
// 2.0 by Peter Polakovic <peter.polakovic@cloudmakers.eu>

/** INDIGO Bus
 \file indigo_io.c
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#if defined(INDIGO_LINUX) || defined(INDIGO_MACOS)
#include <unistd.h>
#include <termios.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#if defined(INDIGO_WINDOWS)
#include <io.h>
#include <winsock2.h>
#define close closesocket
#pragma warning(disable:4996)
#endif

#include "indigo_bus.h"
#include "indigo_io.h"

#if defined(INDIGO_LINUX) || defined(INDIGO_MACOS)

int indigo_open_serial(const char *dev_file) {
	return indigo_open_serial_with_speed(dev_file, 9600);
}

int indigo_open_serial_with_speed(const char *dev_file, int speed) {
	int dev_fd;
	struct termios options;
	if ((dev_fd = open(dev_file, O_RDWR | O_NOCTTY | O_SYNC)) == -1) {
		return -1;
	}
	memset(&options, 0, sizeof options);
	if (tcgetattr(dev_fd, &options) != 0) {
		close(dev_fd);
		return -1;
	}
	cfsetispeed(&options, speed);
	cfsetospeed(&options, speed);
	options.c_lflag &= ~(ICANON|ECHO|ECHOE|ISIG|IEXTEN);
	options.c_oflag &= ~(OPOST);
	options.c_iflag &= ~(INLCR|ICRNL|IXON|IXOFF|IXANY|IMAXBEL);
	options.c_cflag &= ~PARENB;
	options.c_cflag &= ~CSTOPB;
	options.c_cflag &= ~CSIZE;
	options.c_cflag |= CS8;
	options.c_cflag |= CLOCAL;
	options.c_cc[VMIN]  = 0;
	options.c_cc[VTIME] = 50;
	if (tcsetattr(dev_fd,TCSANOW, &options) != 0) {
		close(dev_fd);
		return -1;
	}

	return dev_fd;
}
#endif

int indigo_open_tcp(const char *host, int port) {
	struct sockaddr_in srv_info;
	struct hostent *he;
	int sock;
	struct timeval timeout;
	timeout.tv_sec = 5;
	timeout.tv_usec = 0;
	if ((he = gethostbyname(host))==NULL) {
		return -1;
	}
	if ((sock = socket(AF_INET, SOCK_STREAM, 0))== -1) {
		return -1;
	}
	memset(&srv_info, 0, sizeof(srv_info));
	srv_info.sin_family = AF_INET;
	srv_info.sin_port = htons(port);
	srv_info.sin_addr = *((struct in_addr *)he->h_addr);
	if (connect(sock, (struct sockaddr *)&srv_info, sizeof(struct sockaddr))<0) {
		return -1;
	}
	if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
		close(sock);
		return -1;
	}
	if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
		close(sock);
		return -1;
	}
	return sock;
}

int indigo_open_udp(const char *host, int port) {
	struct sockaddr_in srv_info;
	struct hostent *he;
	int sock;
	struct timeval timeout;
	timeout.tv_sec = 5;
	timeout.tv_usec = 0;
	if ((he = gethostbyname(host))==NULL) {
		return -1;
	}
	if ((sock = socket(AF_INET, SOCK_DGRAM, 0))== -1) {
		return -1;
	}
	memset(&srv_info, 0, sizeof(srv_info));
	srv_info.sin_family = AF_INET;
	srv_info.sin_port = htons(port);
	srv_info.sin_addr = *((struct in_addr *)he->h_addr);
	if (connect(sock, (struct sockaddr *)&srv_info, sizeof(struct sockaddr))<0) {
		return -1;
	}
	if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
		close(sock);
		return -1;
	}
	if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
		close(sock);
		return -1;
	}
	return sock;
}

int indigo_read(int handle, char *buffer, long length) {
	long remains = length;
	long total_bytes = 0;
	while (true) {
#if defined(INDIGO_WINDOWS)
		long bytes_read = recv(handle, buffer, remains, 0);
		if (bytes_read == -1 && WSAGetLastError() == WSAETIMEDOUT) {
			Sleep(500);
			continue;
		}
#else
		long bytes_read = read(handle, buffer, remains);
#endif
		if (bytes_read <= 0) {
			return (int)bytes_read;
		}
		total_bytes += bytes_read;
		if (bytes_read == remains) {
			return (int)total_bytes;
		}
		buffer += bytes_read;
		remains -= bytes_read;
	}
}

#if defined(INDIGO_WINDOWS)
int indigo_recv(int handle, char *buffer, long length) {
	while (true) {
		long bytes_read = recv(handle, buffer, length, 0);
		if (bytes_read == -1 && WSAGetLastError() == WSAETIMEDOUT) {
			Sleep(500);
			continue;
		}
		return (int)bytes_read;
	}
}

int indigo_close(int handle) {
	return closesocket(handle);
}
#endif



int indigo_read_line(int handle, char *buffer, int length) {
	char c = '\0';
	long total_bytes = 0;
	while (total_bytes < length) {
#if defined(INDIGO_WINDOWS)
		long bytes_read = recv(handle, &c, 1, 0);
		if (bytes_read == -1 && WSAGetLastError() == WSAETIMEDOUT) {
			Sleep(500);
			continue;
		}
#else
		long bytes_read = read(handle, &c, 1);
#endif
		if (bytes_read > 0) {
			if (c == '\r')
				;
			else if (c != '\n')
				buffer[total_bytes++] = c;
			else
				break;
		} else {
			errno = ECONNRESET;
			INDIGO_TRACE_PROTOCOL(indigo_trace("%d → ERROR", handle));
			return -1;
		}
	}
	buffer[total_bytes] = '\0';
	INDIGO_TRACE_PROTOCOL(indigo_trace("%d → %s", handle, buffer));
	return (int)total_bytes;
}

bool indigo_write(int handle, const char *buffer, long length) {
	long remains = length;
	while (true) {

#if defined(INDIGO_WINDOWS)
		long bytes_written = send(handle, buffer, remains, 0);
#else
		long bytes_written = write(handle, buffer, remains);
#endif
		if (bytes_written < 0)
			return false;
		if (bytes_written == remains)
			return true;
		buffer += bytes_written;
		remains -= bytes_written;
	}
}

bool indigo_printf(int handle, const char *format, ...) {
	char buffer[1024];
	va_list args;
	va_start(args, format);
	int length = vsnprintf(buffer, sizeof(buffer), format, args);
	va_end(args);
	INDIGO_TRACE_PROTOCOL(indigo_trace("%d ← %s", handle, buffer));
	return indigo_write(handle, buffer, length);
}


int indigo_scanf(int handle, const char *format, ...) {
	char buffer[1024];
	if (indigo_read_line(handle, buffer, sizeof(buffer)) <= 0)
		return 0;
	va_list args;
	va_start(args, format);
	int count = vsscanf(buffer, format, args);
	va_end(args);
	return count;
}
