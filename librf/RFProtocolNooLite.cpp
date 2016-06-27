#include "stdafx.h"
#include "RFProtocolNooLite.h"

static range_type g_timing_pause[7] =
{
	{ 1800, 2200 },
	{ 380, 750 },
	{ 851, 1400 },
	{ 2500, 2700 },
	{ 0,0 }
};  // TODO FIXIT 500us, 1000us

static range_type g_timing_pulse[8] =
{
	{ 1800, 2200 },
	{ 200, 615 },
	{ 815, 1100 },
	{ 2500, 2700 },
	{ 0,0 }
};   // TODO FIXIT 500us, 1000us


CRFProtocolNooLite::CRFProtocolNooLite()
	:CRFProtocol(g_timing_pause, g_timing_pulse, 0, 1, "bBbBbBbBbBbBbBbBbBbBbBa")
{

}


CRFProtocolNooLite::~CRFProtocolNooLite()
{
}


// The 1-Wire CRC scheme is described in Maxim Application Note 27:
// "Understanding and Using Cyclic Redundancy Checks with Maxim iButton Products"

uint8_t crc8(uint8_t *addr, uint8_t len)
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
	unsigned char fmt = (unsigned char)bits2long(bits.substr(bits.length() - 16, 8));
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

	default:
		return false;
	}

	for (int i = 0; i < 4; i++)
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

			if (!bits2packet(res, packet, &packetLen))
				continue;

			unsigned char packetCrc = crc8(packet, packetLen);
			// check crc;

			if (!packetCrc)
			{
				return res;
			}
		}
	}


	return "";
}

string CRFProtocolNooLite::DecodeData(const string& bits) // Преобразование бит в данные
{
	uint8_t packet[20];
	size_t packetLen = sizeof(packet);

	if (!bits2packet(bits, packet, &packetLen))
		return "";
	
	packet[0] >>= 3;
	char buffer[100];
	snprintf(buffer, sizeof(buffer), "cmd=%02x,addr=%04x,fmt=%02x,crc=%02x", (unsigned char)packet[0], (unsigned short)((packet[2] << 8) + packet[1]), (unsigned char)packet[3], (unsigned char)packet[4]);
	return buffer;
}  

bool CRFProtocolNooLite::needDump(const string &rawData)
{
	return rawData.find(m_PacketDelimeter) != rawData.npos;
}
