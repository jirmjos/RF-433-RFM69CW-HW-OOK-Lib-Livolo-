#include "RFProtocolSmartHome.h"


// 
static range_type g_timing_pause[7] =
{
	{ 5400, 18000 }, // Разделитель
	{ 140, 600 }, // Короткий
	{ 800, 1200 }, // Длинный
	{ 0,0 }
};

static range_type g_timing_pulse[8] =
{
	{ 5400, 12000 }, // Разделитель
	{ 140, 600 }, // Короткий
	{ 800, 1200 }, // Длинный
	{ 0,0 }
};

static const uint16_t g_transmit_data[] =
{
	500, 200, 400, 0,  // Pauses
	500, 100, 300, 0   // Pulses
};

CRFProtocolSmartHome::CRFProtocolSmartHome()
	:CRFProtocol(g_timing_pause, g_timing_pulse, 24, 2, "a")
{
	SetTransmitTiming(g_transmit_data);
}



CRFProtocolSmartHome::~CRFProtocolSmartHome()
{
}


string CRFProtocolSmartHome::DecodePacket(const string& raw)
{
	if (raw.length() < 10 )
		return "";

	string res;

	for (int i = 0; i < raw.length()-1; i += 2)
	{
		if (raw.substr(i, 2) == "Cb")
			res += "1";
		else if (raw.substr(i, 2) == "Bc")
			res += "0";
		else
			return "";
	}

	if (raw[raw.length() - 1] == 'B')
		return res;

	return "";
}

string CRFProtocolSmartHome::DecodeData(const string& bits)
{
	int addr = bits2long(bits, 0, 16);
	int cmd = bits2long(bits, 16, 7);

	char buffer[100];
	snprintf(buffer, sizeof(buffer), "addr=%04x cmd=%d", addr, cmd);
	return buffer;
}


string CRFProtocolSmartHome::bits2timings(const string &bits)
{
	string result = bits;

	
	return result;
}

string CRFProtocolSmartHome::data2bits(const string &data)
{
	string proto, dataDetail;
	SplitPair(data, ':', proto, dataDetail);
	if (proto != "SmartHome")
		throw CHaException(CHaException::ErrBadParam, "Bad protocol in '" + data + "'");

	string_map values;
	SplitValues(dataDetail, values);

	string sAddr = values["addr"];
	string sCmd = values["cmd"];

	if (!sAddr.length() || !sCmd.length())
		throw CHaException(CHaException::ErrBadParam, "Bad command for Livolo:" + data);

	uint16_t addr = (uint16_t)strtol(sAddr.c_str(), NULL, 16);
	uint8_t cmd = atoi(sCmd);

	if (!addr || !cmd)
		throw CHaException(CHaException::ErrBadParam, "Bad command for Livolo:" + data);

	string res = reverse(l2bits(addr, 16)) + reverse(l2bits(cmd, 7));

	return res;
}
