#pragma once
#include "RFProtocol.h"
class RFLIB_API CRFProtocolNooLite :
	public CRFProtocol
{ 
	unsigned char getByte(const string &bits, size_t first, size_t len=8);
	bool bits2packet(const string& bits, uint8_t *packet, size_t *packetLen);

public:
	CRFProtocolNooLite();
	~CRFProtocolNooLite();

	virtual string getName() { return "nooLite"; };
	virtual string DecodePacket(const string&);
	virtual string DecodeData(const string&); // Преобразование бит в данные
	virtual bool needDump(const string &rawData);
};

