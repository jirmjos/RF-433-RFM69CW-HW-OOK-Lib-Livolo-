#include "stdafx.h"
#include "RFProtocol.h"


CRFProtocol::CRFProtocol(range_array_type zeroLengths, range_array_type pulseLengths, int bits, int minRepeat, string PacketDelimeter)
:m_ZeroLengths(zeroLengths), m_PulseLengths(pulseLengths), m_Bits(bits), m_MinRepeat(minRepeat), m_PacketDelimeter(PacketDelimeter)
{
	m_Debug = false;
}


CRFProtocol::~CRFProtocol()
{
}

string c2s(char c)
{
	char tmp[2];
	tmp[0] = c;
	tmp[1] = 0;
	return tmp;
}

void CRFProtocol::SetTransmitTiming(const uint16_t *timings)
{
	m_SendTimingPauses = m_SendTimingPulses = timings;
	while (*m_SendTimingPulses++);
}


string CRFProtocol::Parse(base_type* data, size_t dataLen)
{
	Clean();

	string decodedRaw = DecodeRaw(data, dataLen);

	if (!decodedRaw.length())
		return "";

	string_vector rawPackets;
	if (!SplitPackets(decodedRaw, rawPackets))
		return "";

	string bits = DecodeBits(rawPackets);

	if (bits.length())
	{
		string res = getName() + ":" + DecodeData(bits);
		uint8_t tmpBuffer[100];
		size_t tmpBufferSize = sizeof(tmpBuffer);
		EncodeData(res, 2000, tmpBuffer, tmpBufferSize);
		return res;
	}

	if (needDump(decodedRaw))
		m_DumpPacket = true;

	return "";
}
 
/*
string CRFProtocol::tryDecode(string data)
{
	if (data.length() >= m_Bits)
		return getName() + ":" + data;
	else
		return "";
}*/



string CRFProtocol::DecodeRaw(base_type* data, size_t dataLen)
{
	string decodedRaw, decodedRawRev;

	for (size_t i = 0; i < dataLen; i++)
	{
		base_type len = getLengh(data[i]);

		if (isPulse(data[i]))
		{
			int pos = 0;
			for (; m_PulseLengths[pos][0]; pos++)
			{
				if (len >= m_PulseLengths[pos][0] && len <= m_PulseLengths[pos][1])
				{
					decodedRaw += c2s('A' + pos);
					break;
				}
			}

			if (!m_PulseLengths[pos][0])
			{
				if (m_Debug)
					decodedRaw += string("[") + itoa(len) + "}";
				else
					decodedRaw += "?";
			}

			pos = 0;
			for (; m_ZeroLengths[pos][0]; pos++)
			{
				if (len >= m_ZeroLengths[pos][0] && len <= m_ZeroLengths[pos][1])
				{
					decodedRawRev += c2s('a' + pos);
					break;
				}
			}

			if (!m_ZeroLengths[pos][0])
				decodedRawRev += "?";
		}
		else
		{
			int pos = 0;
			for (; m_ZeroLengths[pos][0]; pos++)
			{
				if (len >= m_ZeroLengths[pos][0] && len <= m_ZeroLengths[pos][1])
				{
					decodedRaw += c2s('a' + pos);
					break;
				}
			}

			if (!m_ZeroLengths[pos][0])
			{
				if (m_Debug)
					decodedRaw += string("[") + itoa(len) + "]";
				else
					decodedRaw += "?";
			}

			pos = 0;
			for (; m_PulseLengths[pos][0]; pos++)
			{
				if (len >= m_PulseLengths[pos][0] && len <= m_PulseLengths[pos][1])
				{
					decodedRawRev += c2s('A' + pos);
					break;
				}
			}

			if (!m_PulseLengths[pos][0])
				decodedRawRev += "?";
		}
	}

	return decodedRaw +"?" + decodedRawRev; // TODO Remove and fix RST
}

bool CRFProtocol::SplitPackets(const string &rawData, string_vector& rawPackets)
{
	SplitString(rawData, m_PacketDelimeter, rawPackets);

	return rawPackets.size() > 1;
}

string CRFProtocol::DecodeBits(string_vector&rawPackets)
{
	string res; int count = 0;

	for_each(string_vector, rawPackets, s)
	{
		string packet;
		size_t pos = s->find(m_Debug ? '[' : '?');
		if (pos != string::npos)
			packet = s->substr(0, pos);
		else
			packet = *s;

		if (!packet.length())
			continue;

		string decoded = DecodePacket(packet);

		if (decoded == "")
			continue;

		if (m_Bits && decoded.length() != m_Bits)
			continue;

		if (res.length())
		{
			if (res == decoded)
			{
				if (++count >= m_MinRepeat)
					break;
			}
			else
			{
				res = decoded;
				count = 1;
				if (m_MinRepeat == 1)
					break;
			}
		}
		else
		{
			res = decoded;
			count = 1;
			if (m_MinRepeat == 1)
				break;
		}
	}

	if (res.length() && count >= m_MinRepeat)
		return res;
	else
		return "";
}

string CRFProtocol::DecodePacket(const string &raw)
{
	return raw;
}

string CRFProtocol::DecodeData(const string &raw)
{
	return raw;
}

unsigned long CRFProtocol::bits2long(const string &raw)
{
	unsigned long res=0;
	
	for_each_const(string, raw, i)
	{
		res = res << 1;

		if (*i=='1')
			res++;
	}

	return res;
}

string CRFProtocol::reverse(const string&s)
{
	string res;

	for_each_const(string, s, i)
	{
		res = *i + res;
	}

	return res;
}

string CRFProtocol::ManchesterDecode(const string&raw, bool expectPulse, char shortPause, char longPause, char shortPulse, char longPulse)
{
	enum t_state { expectStartPulse, expectStartPause, expectMiddlePulse, expectMiddlePause };

	t_state state = expectPulse ? expectStartPulse : expectStartPause;
	string res;

	for_each_const(string, raw, c)
	{
		switch (state)
		{
		case expectStartPulse:   // Ожидаем короткий пульс, всегда 1
			if (*c == shortPulse)
			{
				res += expectPulse ? "1":"0";
				state = expectMiddlePause;
			}
			else
			{
				return "";
			}
			break;
		case expectStartPause:  // Ожидаем короткую паузу, всегда 0
			if (*c == shortPause)
			{
				res += expectPulse?"0":"1";
				state = expectMiddlePulse;
			}
			else
			{
				return "";
			}
			break;
		case expectMiddlePulse:  // Ожидаем пульс. Если короткий - пара закончилась, ждем короткую стартовую паузу. Если длинный, получили начало след пары и ждем среднюю паузу
			if (*c == shortPulse)
			{
				state = expectStartPause;
			}
			else if (*c == longPulse)
			{
				state = expectMiddlePause;
				res += expectPulse? "1":"0";
			}
			else
			{
				return "";
			}
			break;
		case expectMiddlePause:  // Ожидаем паузу. Если короткая - пара закончилась, ждем короткую стартовый пульс. Если длинная, получили начало след пары и ждем средний пульс
			if (*c == shortPause)
			{
				state = expectStartPulse;
			}
			else if (*c == longPause)
			{
				state = expectMiddlePulse;
				res += expectPulse?"0":"1";
			}
			else
			{
				return "";
			}
			break;
		default:
			return "";
		}
	}

	return res;
}

string replaceDouble(const string &src, char search, char replace)
{
	string res = src;
	char tmp[3];
	tmp[0] = tmp[1] = search;
	tmp[2] = 0;
	char tmp2[2];
	tmp2[0] = replace;
	tmp2[1] = 0;

	size_t pos;
	while ((pos = res.find(tmp)) != string::npos)
	{
		res = res.replace(pos, 2, tmp2);
	}

	return res;
}

string CRFProtocol::ManchesterEncode(const string&bits, bool invert, char shortPause, char longPause, char shortPulse, char longPulse)
{
	string res;
	for_each_const(string, bits, i)
	{
		bool bit = *i == '1';

		if (bit ^ invert)
		{
			res += "Aa";
			//lastPulse = false;
		}
		else
		{
			res += "aA";
			//lastPulse = true;
		}

	}
	
	res = replaceDouble(res, shortPause, longPause);
	res = replaceDouble(res, shortPulse, longPulse);

	return res;
}


bool CRFProtocol::needDump(const string &rawData)
{
	return false;
}


void CRFProtocol::EncodeData(const string &data, uint16_t bitrate, uint8_t *buffer, size_t &bufferSize)
{
	EncodePacket(data2bits(data), bitrate, buffer, bufferSize);
}

void CRFProtocol::EncodePacket(const string &bits, uint16_t bitrate, uint8_t *buffer, size_t &bufferSize)
{
	string timings = bits2timings(bits);
	uint16_t bitLen = 1000000L / bitrate;
	memset(buffer, 0, bufferSize);

	size_t bitNum = 0;
	for_each (string, timings, i)
	{
		bool pulse = *i < 'a';
		uint16_t len = pulse ? m_SendTimingPulses[*i - 'A'] : m_SendTimingPulses[*i - 'a'];
		uint16_t bits = len / bitLen; // TODO Округление для некратного битрейта?

		for (int j = 0; j < bits; j++)
		{
			if (pulse)
				buffer[bitNum >> 3] |= (1 << (7-(bitNum & 7)));

			bitNum++;
		}
	}

	bufferSize = (bitNum+7)>>3;
}


string CRFProtocol::bits2timings(const string &bits)
{
	throw CHaException(CHaException::ErrNotImplemented, "CRFProtocol::bits2timings");
}

string CRFProtocol::data2bits(const string &data)
{
	throw CHaException(CHaException::ErrNotImplemented, "CRFProtocol::data2bits");
}
