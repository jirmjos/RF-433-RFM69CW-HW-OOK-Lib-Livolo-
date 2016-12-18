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
	virtual string DecodeData(const string&); // Преобразование бит в данные

};

