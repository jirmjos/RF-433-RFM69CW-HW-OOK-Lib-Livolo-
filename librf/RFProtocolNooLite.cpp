#include "stdafx.h"
#include "RFProtocolNooLite.h"

static range_type g_timing_pause[7] =
{
	{ 1800, 2300 },
	{ 380, 750 },
	{ 851, 1400 },
	{ 2500, 2700 },
	{ 0,0 }
};  // TODO FIXIT 500us, 1000us

static range_type g_timing_pulse[8] =
{
	{ 1800, 2300 },
	{ 200, 650 },
	{ 700, 1100 },
	{ 2500, 2700 },
	{ 0,0 }
};   // TODO FIXIT 500us, 1000us


CRFProtocolNooLite::CRFProtocolNooLite()
	:CRFProtocol(g_timing_pause, g_timing_pulse, 0, 1, "bBbBbBbBbBbBbBbBbBbBbBa")
{
	m_Debug = true;
}


CRFProtocolNooLite::~CRFProtocolNooLite()
{
}


// The 1-Wire CRC scheme is described in Maxim Application Note 27:
// "Understanding and Using Cyclic Redundancy Checks with Maxim iButton Products"

uint8_t CRFProtocolNooLite::crc8(uint8_t *addr, uint8_t len)
{
	uint8_t crc = 0;

	for (uint8_t i = 0; i<len; i++)
	{
		uint8_t inbyte = addr[i];
		for (uint8_t j = 0; j<8; j++)
		{
			uint8_t mix = (crc ^ inbyte) & 0x01;
			crc >>= 1;
			if (mix)
				crc ^= 0x8C;

			inbyte >>= 1;
		}
	}
	return crc;
}


unsigned char CRFProtocolNooLite::getByte(const string &bits, size_t first, size_t len)
{
	return (unsigned char)bits2long(reverse(bits.substr(first, len)));
}

bool CRFProtocolNooLite::bits2packet(const string& bits, uint8_t *packet, size_t *packetLen)
{
	 
	size_t bytes = (bits.substr(1).length()+7) / 8;
	size_t extraBits = bits.substr(1).length() % 8;

	if (*packetLen < bytes)
		return false;

	string reverseBits = reverse(bits.substr(1));
	while (reverseBits.length() % 8)
		reverseBits += '0';

	for (unsigned int i = 0; i < bytes; i++)
	{
		packet[bytes - 1 - i] = (uint8_t)bits2long(reverseBits.substr(i * 8, 8));// min(8, reverseBits.length() - i * 8)));
	}

	uint8_t packetCrc = crc8(packet, (uint8_t)bytes);
	*packetLen = bytes;

	return packetCrc==0;

	unsigned char fmt = (unsigned char)bits2long(reverse(bits.substr(bits.length() - 16, 8)));
	*packetLen = 0;
	packet[0] = getByte(bits, 1, 5) << 3;
	size_t len = bits.length();

	switch (fmt)
	{
	case 0:
		if (len != 38)
			return false;
		*packetLen = 5;
		break;

	case 7: //?
 		if (len != 74)
			return false;
		*packetLen = 9;
		break;

	default:
		return false;
	}

	for (unsigned int i = 0; i < *packetLen-1; i++)
	{
		packet[*packetLen-i-1] = getByte(bits, len - 8*(i+1));
	}

	return true;
}


string CRFProtocolNooLite::DecodePacket(const string& raw)
{
	if (raw.length() < 10 )
		return "";

	string_vector v;
	SplitString(raw, 'd', v);

	if (v.size()==1)
		SplitString(raw, 'a', v);   // TODO CheckIT

	string res;

	for_each(string_vector, v, i)
	{
		res = ManchesterDecode('b' + *i, false, 'b', 'c', 'B', 'C');

		if (res.length() >=37)
		{
			
			uint8_t packet[20];
			size_t packetLen = sizeof(packet);

			if (bits2packet(res, packet, &packetLen))
				return res;
			/*  TODO
			unsigned char packetCrc = crc8(packet, packetLen);
			// check crc;

			if (!packetCrc)
			{
				return res;
			}*/
		}
	}


	return "";
}

string CRFProtocolNooLite::DecodeData(const string& bits) // Преобразование бит в данные
{
	uint8_t packet[20];
	size_t packetLen = sizeof(packet);

	if (!bits2packet(bits, packet, &packetLen))
		return bits;
	
	if (packetLen < 5)
		return bits;

	char buffer[100]="";
	uint8_t fmt = packet[packetLen - 2];
	switch (fmt)
	{
	case 0:
		{
			bool sync = (packet[0] & 8) != 0;
			uint8_t cmd =packet[0] >> 4;
			snprintf(buffer, sizeof(buffer), "sync=%d cmd=%d addr=%04x fmt=%02x crc=%02x", sync, cmd, (uint16_t)((packet[2] << 8) + packet[1]), (uint8_t)packet[3], (uint8_t)packet[4]);
			break;
		}
	case 1:
		{
			uint8_t type = packet[3];
			snprintf(buffer, sizeof(buffer), "cmd=%02x b0=%02x type=%d addr=%04x fmt=%02x crc=%02x", (uint8_t)packet[0], (uint8_t)packet[1],
				type, 
				(uint16_t)((packet[packetLen - 3] << 8) + packet[packetLen - 4]), (uint8_t)fmt, (uint8_t)packet[packetLen - 1]);
		}
		break;

	case 7:
		if (packet[1]==21)
		{
			uint8_t type = (packet[3] >> 4) & 7;
			int t0 = ((packet[3] & 0x7) << 8) | packet[2];
			float t = (float)0.1*((packet[3]&8)?4096-t0:t0);
			int h = packet[4];
			int s3 = packet[5];
			bool bat = (packet[3] & 0x80) != 0;
			if (type==2)
			{
				snprintf(buffer, sizeof(buffer), "sync=%02x cmd=%d type=%d t=%.1f h=%d s3=%02x bat=%d addr=%04x fmt=%02x crc=%02x", (uint8_t)packet[0], (uint8_t)packet[1], 
					type, t, h, s3, bat,
					(uint16_t)((packet[packetLen - 3] << 8) + packet[packetLen - 4]), (uint8_t)fmt, (uint8_t)packet[packetLen - 1]);
			} else if (type==3)
			{
				snprintf(buffer, sizeof(buffer), "sync=%02x cmd=%d type=%d t=%.1f s3=%02x bat=%d addr=%04x fmt=%02x crc=%02x", (uint8_t)packet[0], (uint8_t)packet[1], 
					type, t, s3, bat,
					(uint16_t)((packet[packetLen - 3] << 8) + packet[packetLen - 4]), (uint8_t)fmt, (uint8_t)packet[packetLen - 1]);
			} else
			{
				snprintf(buffer, sizeof(buffer), "sync=%02x cmd=%02x type=%02x b3=%02x b4=%02x b5=%02x addr=%04x fmt=%02x crc=%02x", (uint8_t)packet[0], (uint8_t)packet[1], (uint8_t)packet[2], (uint8_t)packet[3], (uint8_t)packet[4], (uint8_t)packet[5], (uint16_t)((packet[7] << 8) + packet[6]), (uint8_t)packet[8], (uint8_t)packet[9]);
			}

		}
		else
		{
			snprintf(buffer, sizeof(buffer), "cmd=%02x b1=%02x b2=%02x b3=%02x b4=%02x b5=%02x addr=%04x fmt=%02x crc=%02x", (uint8_t)packet[0], (uint8_t)packet[1], (uint8_t)packet[2], (uint8_t)packet[3], (uint8_t)packet[4], (uint8_t)packet[5], (uint16_t)((packet[7] << 8) + packet[6]), (uint8_t)packet[8], (uint8_t)packet[9]);
		}
		break;

<<<<<<< HEAD
	default: 
		m_Log->Printf(3, "len=%d addr=%04x fmt=%02x crc=%02x", packetLen, (uint16_t)((packet[packetLen - 3] << 8) + packet[packetLen - 4]), (uint8_t)fmt, (uint8_t)packet[packetLen - 1]);
		m_Log->PrintBuffer(3, packet, packetLen);
=======
	default:
		m_Log->PrintBuffer(3, packet, packetLen);
		snprintf(buffer, sizeof(buffer), "len=%d,addr=%04x,fmt=%02x,crc=%02x", packetLen, (uint16_t)((packet[packetLen - 3] << 8) + packet[packetLen - 4]), (uint8_t)fmt, (uint8_t)packet[packetLen - 1]);
>>>>>>> master
	}

	return buffer;
}  

bool CRFProtocolNooLite::needDump(const string &rawData)
{
	return rawData.find(m_PacketDelimeter) != rawData.npos;
}
