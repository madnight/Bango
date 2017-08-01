#include "CSocket.h"

bool CSocket::g_bDebug = false;

char* CSocket::WriteV(char* packet, va_list va)
{
	char* format = va_arg(va, char*);

	for ( ; ; ) {
		switch (*format++) {

		case 'b':
		case 'B':
			*packet++ = va_arg(va, int);
			break;
		case 'w':
		case 'W':
			*(WORD *)packet = va_arg(va, int);
			packet += sizeof(WORD);
			break;
		case 'd':
			*(DWORD *)packet = va_arg(va, DWORD);
			packet += sizeof(DWORD);
			break;
		case 'U':
			*(UINT *)packet = va_arg(va, UINT);
			packet += sizeof(UINT);
			break;
		case 'I':	//	__int64
			*(__int64 *)packet = va_arg(va, __int64);
			packet += sizeof(__int64);
			break;
		case 's':
		case 'S':
			{
				char *str = va_arg(va, char *);
				if (str == 0 || *str == 0) {
					*packet++ = 0;
					break;
				}
				for ( ; (*packet++ = *str++ ) != 0 ; )
					;
			}
			break;
		case 'm':
		case 'M':
			{
				char *ptr = va_arg(va, char *);
				int	size = va_arg(va, int);
				memcpy(packet, ptr, size);
				packet += size;
			}
			break;
		case 0:
		default:
			return packet;
		}
	}

	return packet;
}

char* CSocket::WritePacket(char* packet, const char* format, ...)
{
	va_list va;
	va_start(va, format);

	char byArgNum = strlen(format);
	for (char i = 0; i < byArgNum; i++)
	{
		switch (format[i])
		{
		case 'b':
		case 'B':
			*packet++ = va_arg(va, int);
			break;
		case 'w':
		case 'W':
			*(WORD *)packet = va_arg(va, int);
			packet += sizeof(WORD);
			break;
		case 'd':
			*(DWORD *)packet = va_arg(va, DWORD);
			packet += sizeof(DWORD);
			break;
		case 'U':
			*(UINT *)packet = va_arg(va, UINT);
			packet += sizeof(UINT);
			break;
		case 'I':	//	__int64
			*(__int64 *)packet = va_arg(va, __int64);
			packet += sizeof(__int64);
			break;
		case 's':
		case 'S':
			{
				char *str = va_arg(va, char *);
				if (str == 0 || *str == 0) {
					*packet++ = 0;
					break;
				}
				for ( ; (*packet++ = *str++ ) != 0 ; )
					;
			}
			break;
		case 'm':
		case 'M':
			{
				char *ptr = va_arg(va, char *);
				int	size = va_arg(va, int);
				memcpy(packet, ptr, size);
				packet += size;
			}
			break;
		case 0:
		default:
			return packet;
		}
	}

	va_end(va);
	return packet;
}

char* CSocket::WritePacketFromFile(char* packet, const char* fileName)
{
	std::ifstream f(fileName);
	char *p = packet;

	std::string line;
	while (std::getline(f, line))
	{
		if (line.empty() || line[0] == ';') continue;

		std::istringstream iss(line);

		char format[2]={0,};

		//if (!(iss >> format[0] >> value)) continue;

		iss >> format[0];
		if (format[0] == 's') {
			std::string value;
			iss >> value;
			p = WritePacket(p, format, value.c_str());
		} else {
			__int64 value=0;
			iss >> value;
			p = WritePacket(p, format, value);
		}
	}

	return p;
}

char* CSocket::ReadPacket(char* packet, const char* format, ...)
{
	va_list va;
	va_start(va, format);

	char byArgNum = strlen(format);
	for (char i = 0; i < byArgNum; i++)
	{
		switch (format[i])
		{
		case 'b':
			*va_arg(va, char*) = *packet;
			packet++;
			break;
		case 'd':
			*va_arg(va, DWORD*) = *(DWORD*)packet;
			packet += sizeof(DWORD);
			break;
		case 'w':
			*va_arg(va, WORD*) = *(WORD*)packet;
			packet += sizeof(WORD);
			break;
		case 'U':
			*va_arg(va, UINT*) = *(UINT*)packet;
			packet += sizeof(UINT);
			break;
		case 'I':
			*va_arg(va, __int64*) = *(__int64*)packet;
			packet += sizeof(__int64);
			break;
		case 's':
			{
                // ą®ŔÚŔÇ ÁÖĽŇ¸¦ ŔúŔĺÇŃ´Ů.
                char *text = *va_arg(va, char**) = packet;
                //packet += sizeof(char*);

                // ą®ŔÚ´Â şńżöÁŘ´Ů.
                while (*packet++) {}
            }
            break;
		case 0:
		default:
			return packet;
		}
	}

	va_end(va);
	return packet;
}
