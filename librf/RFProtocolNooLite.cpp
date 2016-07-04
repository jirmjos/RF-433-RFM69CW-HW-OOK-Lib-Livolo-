#include "stdafx.h"
#include "RFProtocolNooLite.h"

static const range_type g_timing_pause[7] =
{
	{ 380, 750 },
	{ 851, 1400 },
	{ 1800, 2300 },
	{ 2500, 2700 },
	{ 0,0 }
};  // TODO FIXIT 500us, 1000us

static const range_type g_timing_pulse[8] =
{
	{ 200, 650 },
	{ 700, 1100 },
	{ 1800, 2300 },
	{ 2500, 2700 },
	{ 0,0 }
};   // TODO FIXIT 500us, 1000us

static const uint16_t g_transmit_data[]=
{
	500, 1000, 1500, 2000, 0,  // Pauses
	500, 1000, 1500, 2000, 0   // Pulses
};


static const char* g_nooLite_Commands[]=
{
	"off",              //0 – выключить нагрузку
	"slowdown",			//1 – запускает плавное понижение яркости
	"on",				//2 – включить нагрузку
	"slowup",			//3 – запускает плавное повышение яркости
	"switch",			//4 – включает или выключает нагрузку
	"slowswitch",		//5 – запускает плавное изменение яркости в обратном
	"shadow_level",		//6 – установить заданную в «Данные к команде_0» яркость
	"callscene",		//7 – вызвать записанный сценарий
	"recordscene",		//8 – записать сценарий
	"unbind",			//9 – запускает процедуру стирания адреса управляющего устройства из памяти исполнительного
	"slowstop",			//10 – остановить регулировку
	"?11",				//значения 11, 12, 13, 14– зарезервированы, не используются
	"?12",				//
	"?13",				//
	"?14",				//
	"bind",				//15 – сообщает, что устройство хочет записать свой адрес в память
	"slowcolor",		//16 – включение плавного перебора цвета
	"switchcolor",		//17 – переключение цвета
	"switchmode",		//18 – переключение режима работы
	"switchspeed",		//19 – переключение скорости эффекта для режима работы
	"battery_low",		//20 – информирует о разряде батареи в устройстве
	"temperature",		//21 – передача информации о текущей температуре и
						//     влажности (Информация о температуре и влажности содержится в
						//     поле «Данные к команде_x».)
};


CRFProtocolNooLite::CRFProtocolNooLite()
	:CRFProtocol(g_timing_pause, g_timing_pulse, 0, 1, "aAaAaAaAaAaAaAaAaAaAaAc")
{
	m_Debug = false;
<<<<<<< HEAD
	SetTransmitTiming(g_transmit_data);
=======
	SetSendTiming(g_transmit_data);


	string tmp;
	tmp = ManchesterEncode("11111", true, 'a', 'b', 'A', 'B');
	tmp = ManchesterEncode("00000", true, 'a', 'b', 'A', 'B');
	tmp = ManchesterEncode("111000", true, 'a', 'b', 'A', 'B');
	tmp = ManchesterEncode("10101010", true, 'a', 'b', 'A', 'B');

>>>>>>> d2755d24ec586bdec1bc1dc7d402c870cd81660e
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

bool CRFProtocolNooLite::bits2packet(const string& bits, uint8_t *packet, size_t *packetLen, uint8_t *CRC)
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

	if (CRC)
		*CRC = packetCrc;

	return packetCrc==0;
}


string CRFProtocolNooLite::DecodePacket(const string& raw)
{
	if (raw.length() < 10 )
		return "";

	string_vector v;
	SplitString(raw, 'd', v);

	if (v.size()==1)
		SplitString(raw, 'c', v);   // TODO CheckIT

	string res;

	for_each(string_vector, v, i)
	{
		res = ManchesterDecode('a' + *i, false, 'a', 'b', 'A', 'B');
<<<<<<< HEAD
		string tmp = bits2timings(res);
=======
		string tmp = EncodePacket(res);
>>>>>>> d2755d24ec586bdec1bc1dc7d402c870cd81660e
		uint8_t tmpBuffer[100];
		size_t tmpBufferSize = sizeof(tmpBuffer);
		EncodeData(res, 2000, tmpBuffer, tmpBufferSize);

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

string CRFProtocolNooLite::DecodeData(const string& bits) // Ïðåîáðàçîâàíèå áèò â äàííûå
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
			snprintf(buffer, sizeof(buffer), "flip=%d cmd=%d addr=%04x fmt=%02x crc=%02x", sync, cmd, (uint16_t)((packet[2] << 8) + packet[1]), (uint8_t)packet[3], (uint8_t)packet[4]);
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
				snprintf(buffer, sizeof(buffer), "flip=%d cmd=%d type=%d t=%.1f h=%d s3=%02x bat=%d addr=%04x fmt=%02x crc=%02x", (uint8_t)packet[0]?1:0, (uint8_t)packet[1], 
					type, t, h, s3, bat,
					(uint16_t)((packet[packetLen - 3] << 8) + packet[packetLen - 4]), (uint8_t)fmt, (uint8_t)packet[packetLen - 1]);
			} else if (type==3)
			{
				snprintf(buffer, sizeof(buffer), "flip=%d cmd=%d type=%d t=%.1f s3=%02x bat=%d addr=%04x fmt=%02x crc=%02x", (uint8_t)packet[0] ? 1 : 0, (uint8_t)packet[1],
					type, t, s3, bat,
					(uint16_t)((packet[packetLen - 3] << 8) + packet[packetLen - 4]), (uint8_t)fmt, (uint8_t)packet[packetLen - 1]);
			} else
			{
				snprintf(buffer, sizeof(buffer), "flip=%d cmd=%02x type=%02x b3=%02x b4=%02x b5=%02x addr=%04x fmt=%02x crc=%02x", (uint8_t)packet[0] ? 1 : 0, (uint8_t)packet[1], (uint8_t)packet[2], (uint8_t)packet[3], (uint8_t)packet[4], (uint8_t)packet[5], (uint16_t)((packet[7] << 8) + packet[6]), (uint8_t)packet[8], (uint8_t)packet[9]);
			}

		}
		else
		{
			snprintf(buffer, sizeof(buffer), "cmd=%02x b1=%02x b2=%02x b3=%02x b4=%02x b5=%02x addr=%04x fmt=%02x crc=%02x", (uint8_t)packet[0], (uint8_t)packet[1], (uint8_t)packet[2], (uint8_t)packet[3], (uint8_t)packet[4], (uint8_t)packet[5], (uint16_t)((packet[7] << 8) + packet[6]), (uint8_t)packet[8], (uint8_t)packet[9]);
		}
		break;

	default: 
		m_Log->Printf(3, "len=%d addr=%04x fmt=%02x crc=%02x", packetLen, (uint16_t)((packet[packetLen - 3] << 8) + packet[packetLen - 4]), (uint8_t)fmt, (uint8_t)packet[packetLen - 1]);
		m_Log->PrintBuffer(3, packet, packetLen);
	}

	return buffer;
}  

bool CRFProtocolNooLite::needDump(const string &rawData)
{
	return rawData.find(m_PacketDelimeter) != rawData.npos;
}


<<<<<<< HEAD
string CRFProtocolNooLite::bits2timings(const string &bits)
=======
string CRFProtocolNooLite::EncodePacket(const string &bits)
>>>>>>> d2755d24ec586bdec1bc1dc7d402c870cd81660e
{
	string start;
	for (int i = 0; i < 37; i++)
	{
		start += '1';
	}

<<<<<<< HEAD
	return 'b'+ManchesterEncode(start, true, 'a', 'b', 'A', 'B')
=======
	return ManchesterEncode(start, true, 'a', 'b', 'A', 'B')
>>>>>>> d2755d24ec586bdec1bc1dc7d402c870cd81660e
		+ 'b' + ManchesterEncode(bits, true, 'a', 'b', 'A', 'B')
		+ 'b' + ManchesterEncode(bits, true, 'a', 'b', 'A', 'B');
}

<<<<<<< HEAD
string l2bits(uint16_t val, int bits)
{
	string res;
	for (int i=0;i<bits;i++)
	{
		res = ((val&1)?'1':'0')+res;
		val>>=1;
	}

	return res;
}

string CRFProtocolNooLite::data2bits(const string &data)
{
	string proto, dataDetail;
	SplitPair(data, ':', proto, dataDetail);
	if (proto!="nooLite")
		throw CHaException(CHaException::ErrBadParam, "Bad protocol in '"+data+"'");

	string_map values;
	SplitValues(dataDetail, values);

	string sAddr = values["addr"];
	string sCmd = values["cmd"];
	string sFmt = values["fmt"];
	string sFlip = values["flip"];

	uint16_t addr = (uint16_t)strtol(sAddr.c_str(), NULL, 16);
	uint8_t cmd = atoi(sCmd);
	uint8_t fmt = sFmt.length()?atoi(sFmt):0xFF;
	bool flip = sFlip.length()?(bool)atoi(sFlip):!m_lastFlip[addr];
	m_lastFlip[addr] = flip;
	int extraBytes = 0;
	string res;

	switch (cmd)
	{
		case nlcmd_off:             //0 – выключить нагрузку
		case nlcmd_slowdown:		//1 – запускает плавное понижение яркости
		case nlcmd_on:				//2 – включить нагрузку
		case nlcmd_slowup:			//3 – запускает плавное повышение яркости
		case nlcmd_switch:			//4 – включает или выключает нагрузку
		case nlcmd_slowswitch:		//5 – запускает плавное изменение яркости в обратном
		case nlcmd_bind:			//15 – сообщает, что устройство хочет записать свой адрес в память
		case nlcmd_unbind:			//9 – запускает процедуру стирания адреса управляющего устройства из памяти исполнительного
			if (fmt!=0)
				throw CHaException(CHaException::ErrBadParam, "bad format: "+data);
			res = l2bits(flip,1)+l2bits(cmd,4)+l2bits(addr,16)+l2bits(fmt,8), l2bits(0/*crc*/,8);
			break;


/*
		nlcmd_shadow_level,		//6 – установить заданную в «Данные к команде_0» яркость
		nlcmd_callscene,		//7 – вызвать записанный сценарий
		nlcmd_recordscene,		//8 – записать сценарий
		nlcmd_slowstop,			//10 – остановить регулировку
		nlcmd_slowcolor,		//16 – включение плавного перебора цвета
		nlcmd_switchcolor,		//17 – переключение цвета
		nlcmd_switchmode,		//18 – переключение режима работы
		nlcmd_switchspeed,		//19 – переключение скорости эффекта для режима работы
		nlcmd_battery_low,		//20 – информирует о разряде батареи в устройстве
		nlcmd_temperature,		//21 – передача информации о текущей температуре и
*/
		default:
				throw CHaException(CHaException::ErrBadParam, "usupported cmd: "+data);
	}

	uint8_t packet[100];
	size_t packetLen = sizeof(packet);
	uint8_t crc;
	bits2packet(res, packet, &packetLen, &crc);
	res = res.substr(res.length()-8)+l2bits(crc,8);

	return res;
}


=======
>>>>>>> d2755d24ec586bdec1bc1dc7d402c870cd81660e
