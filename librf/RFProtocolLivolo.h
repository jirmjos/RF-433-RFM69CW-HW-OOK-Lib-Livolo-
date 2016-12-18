#pragma once
#include "RFProtocol.h"
class RFLIB_API CRFProtocolLivolo :
	public CRFProtocol
{
public:
	CRFProtocolLivolo();
	virtual ~CRFProtocolLivolo();

	virtual string getName() { return "Livolo"; };
	virtual string DecodePacket(const string&);

	// Кодирование
	virtual string bits2timings(const string &bits);
	virtual string data2bits(const string &data);
};

