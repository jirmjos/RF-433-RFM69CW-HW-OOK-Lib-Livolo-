#pragma once
#include "stdafx.h"
#include "rflib.h"

typedef unsigned long base_type;

#define PULSE_BIT       0x01000000
#define PULSE_MASK      0x00FFFFFF

typedef  base_type range_type[2];
typedef  const range_type *range_array_type;
string c2s(char c);

class RFLIB_API CRFProtocol
{
protected:
	range_array_type m_ZeroLengths, m_PulseLengths;
	int m_MinRepeat, m_Bits;
	string m_PacketDelimeter;
	bool m_Debug, m_DumpPacket;
	CLog *m_Log;
	const uint16_t *m_SendTimingPauses;
	const uint16_t *m_SendTimingPulses;

	string ManchesterDecode(const string&, bool expectPulse, char shortPause, char longPause, char shortPulse, char longPulse);
	string ManchesterEncode(const string&, bool invert, char shortPause, char longPause, char shortPulse, char longPulse);
	virtual void Clean() { m_DumpPacket = false; };
	virtual bool needDump(const string &rawData);
<<<<<<< HEAD
	void SetTransmitTiming(const uint16_t *timings);
=======
	void SetSendTiming(const uint16_t *timings);
>>>>>>> d2755d24ec586bdec1bc1dc7d402c870cd81660e

public:

	CRFProtocol(range_array_type zeroLengths, range_array_type pulseLengths, int bits, int minRepeat, string PacketDelimeter );
	virtual ~CRFProtocol();

	// Раскодируем пакет
	virtual string Parse(base_type*, size_t len);
	virtual string DecodeRaw(base_type* data, size_t dataLen);  // Декодироавние строки по длинам
	virtual bool SplitPackets(const string &rawData, string_vector& rawPackets); // Нарезка по пакетам
	virtual string DecodeBits(string_vector&rawPackets); // Сборка бит по массиву пакетов
	virtual string DecodePacket(const string&); // Преобразование строки длин в биты
	virtual string DecodeData(const string&); // Преобразование бит в данные
//	virtual string tryDecode(string data);

	// Кодируем пакет
	virtual void EncodeData(const string &data, uint16_t bitrate,  uint8_t *buffer, size_t &bufferSize);
	virtual void EncodePacket(const string &bits, uint16_t bitrate, uint8_t *buffer, size_t &bufferSize);
<<<<<<< HEAD
	virtual string bits2timings(const string &bits);
	virtual string data2bits(const string &data);
=======
	virtual string EncodePacket(const string &bits);
>>>>>>> d2755d24ec586bdec1bc1dc7d402c870cd81660e

	static inline bool isPulse(base_type v) { return (v&PULSE_BIT) != 0; };
	static inline base_type getLengh(base_type v) { return v&PULSE_MASK; };
	virtual string getName() = 0;
	bool needDumpPacket() {	return m_DumpPacket; };

	unsigned long bits2long(const string&); 
	string reverse(const string&);
	void setLog(CLog *log) { m_Log = log; };
};

