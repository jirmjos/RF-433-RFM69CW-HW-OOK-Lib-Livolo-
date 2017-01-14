#include "stdafx.h"
#include "RFProtocolLivolo.h"
#include "../libutils/strutils.h"

// 
static range_type g_timing_pause[7] =
{
	{ 420, 570 }, // Разделитель
	{ 140, 280 }, // Короткий
	{ 290, 440 }, // Длинный
	{ 0,0 }
};

static range_type g_timing_pulse[8] =
{
	{ 380, 500 },
	{  20, 170 },
	{ 190, 300 },
//	{ 200000, 200001 },
	{ 0,0 }
};

static const uint16_t g_transmit_data[]=
{
	500, 100, 300, 0,  // Pauses
	500, 100, 300, 0   // Pulses
};



/*  1234567890123456 1234567
	0011 0011 1101 0001  000 100 0 - A
	0011 0011 1101 0001  001 000 0 - B
	0011 0011 1101 0001  011 100 0 - C
	0011 0011 1101 0001  010 101 0 - D
*/

/*
0110 1010 0100 = 06A4
1 - 000 000 0
2 - 110 000 0
3 - 111 100 0
4 - 001 100 0
5 - 110 110 0
+ - 101 110 0 !
- - 111 010 0 !
I - 101 101 0 !
II- 111 001 0 !

1 - 101 000 0
2 - 011 000 0
3 - 000 110 0
4 - 010 100 0
5 - 100 100 0
+ - 111 111 0 !
- - 010 010 0 !
I - 000 101 0 !
II- 001 001 0 !

C - 110 1010
*/


CRFProtocolLivolo::CRFProtocolLivolo()
	:CRFProtocol(g_timing_pause, g_timing_pulse, 23, 2, "A")
{
	SetTransmitTiming(g_transmit_data);
}


CRFProtocolLivolo::~CRFProtocolLivolo()
{
}

string CRFProtocolLivolo::DecodePacket(const string&packet)
{
	string bits; bool waitSecondShort = false;

	for_each_const(string, packet, s)
	{
		if (waitSecondShort)
		{
			waitSecondShort = false;
			if (*s == 'b' || *s == 'B')
				continue;
			else
				return "";

		}

		if (*s == 'b' || *s == 'B')
		{
			bits += "0";
			waitSecondShort = true;
		}
		else if (*s == 'c' || *s == 'C')
		{
			bits += "1";
		}
	}

	return bits;
}

string CRFProtocolLivolo::DecodeData(const string& bits)
{
	int addr = bits2long(bits, 0, 16);
	int cmd = bits2long(bits, 16, 7);

	char buffer[100];
	snprintf(buffer, sizeof(buffer), "addr=%04x cmd=%d", addr, cmd);
	return buffer;
}

// Кодирование
string selectPulse(bool inBit, bool &high) 
{
	string result;

	if (inBit) 
	{
		if (high == true) { // if current pulse should be high, send High One
			result += 'c';
		}
		else {             // else send Low One
			result += 'C';
		}
		high = !high; // invert next pulse
	} else {
		for (int i = 0; i<2; i++) {
			if (high == true) {   // if current pulse should be high, send High Zero
				result += 'b';
			}
			else {              // else send Low Zero
				result += 'B';
			}
			high = !high; // invert next pulse
		}
	}

	return result;
}

string CRFProtocolLivolo::bits2timings(const string &bits)
{
	string result;

	for (int pulse= 0; pulse <= 180; pulse = pulse+1) { // how many times to transmit a command
		result+="A";
		bool high = true; // first pulse is always high
		for_each_const(string, bits, i)
			result+=selectPulse(*i=='1', high);    
	}

	return result;
}

string CRFProtocolLivolo::data2bits(const string &data)
{
	string proto, dataDetail;
	SplitPair(data, ':', proto, dataDetail);
	if (proto != "Livolo")
		throw CHaException(CHaException::ErrBadParam, "Bad protocol in '" + data + "'");

	string_map values;
	SplitValues(dataDetail, values);

	string sAddr = values["addr"];
	string sCmd = values["cmd"];

	if (!sAddr.length() || !sCmd.length())
		throw CHaException(CHaException::ErrBadParam, "Bad command for Livolo:"+data);

	uint16_t addr = (uint16_t)strtol(sAddr.c_str(), NULL, 16);
	uint8_t cmd = atoi(sCmd);

	if (!addr || !cmd)
		throw CHaException(CHaException::ErrBadParam, "Bad command for Livolo:" + data);

	string res = reverse(l2bits(addr, 16)) + reverse(l2bits(cmd, 7));

	return res;
}
