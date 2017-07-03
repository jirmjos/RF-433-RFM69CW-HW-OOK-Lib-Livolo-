#pragma once
#include "RFProtocol.h"
class RFLIB_API CRFProtocolSmartHome :
	public CRFProtocol
{
public:
	CRFProtocolSmartHome();
	~CRFProtocolSmartHome();

	virtual string getName() { return "SmartHome"; };
	virtual string DecodePacket(const string&);
	virtual string DecodeData(const string&); 

											  
	virtual string bits2timings(const string &bits);
	virtual string data2bits(const string &data);

};

