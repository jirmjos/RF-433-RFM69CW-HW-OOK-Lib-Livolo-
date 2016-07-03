#pragma once
#include "RFProtocol.h"
class RFLIB_API CRFProtocolNooLite :
	public CRFProtocol
{ 
	unsigned char getByte(const string &bits, size_t first, size_t len=8);
	bool bits2packet(const string& bits, uint8_t *packet, size_t *packetLen);
	uint8_t crc8(uint8_t *addr, uint8_t len);

public:
	CRFProtocolNooLite();
	~CRFProtocolNooLite();

	virtual string getName() { return "nooLite"; };
	virtual string DecodePacket(const string&);
	virtual string DecodeData(const string&); // Преобразование бит в данные
	virtual bool needDump(const string &rawData);
	
	// Кодирование
	virtual string EncodePacket(const string &bits);


};

