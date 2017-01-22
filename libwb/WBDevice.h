#pragma once
//#include "../engine/StateJob.h"
#include "libwb.h"
#include "../libutils/strutils.h"

struct LIBWB_API CWBControl
{
	enum ControlType
	{
		Error=0,
		Switch,  //0 or 1
		Alarm, // 
		PushButton, // 1
		Range, // 0..255 [TODO] - max value
		Rgb, 
		Text,
		Generic,
		Temperature, //	temperature	°C	float
		RelativeHumidity, //	rel_humidity	%, RH	float, 0 - 100
		AtmosphericPressure, //	atmospheric_pressure	millibar(100 Pa)	float
		SoundLevel,
		PrecipitationRate, //(rainfall rate)	rainfall	mm per hour	float
		WindSpeed, //	wind_speed	m / s	float
		PowerPower, //	watt	float
		PowerConsumption, //	power_consumption	kWh	float
		VoltageVoltage, //	volts	float
		WaterFlow, //	water_flow	m ^ 3 / hour	float
		WaterTotal, // consumption	water_consumption	m ^ 3	float
		Resistance, //	resistance	Ohm	float
		GasConcentration //	concentration	ppm	float(unsigned)

	};

	ControlType Type;
	string Name, Source, SourceType;
	bool Readonly, Changed;
	string sValue;
	float fValue;
	int Max;
	CWBControl(const string &name);
	void enrich(const string &meta, const string &val);
	static ControlType getType(const string & type);
	static string getTypeName(ControlType type);
	ControlType getType();
	string getTypeName();
	void setType(const string& type);
	bool isLoaded();
};

typedef map<string, CWBControl*> CControlMap;

class LIBWB_API CWBDevice
{
	string m_Name;
	string m_Description;
	CControlMap m_Controls;

public:
	CWBDevice(string Name, string Description);
	//CWBDevice();
	~CWBDevice();

	string getName(){return m_Name;};
	string getDescription(){return m_Description;};
#ifdef USE_CONFIG
	void Init(CConfigItem config);
#endif	
	void addControl(const string &Name);
	void addControl(const string &Name, CWBControl::ControlType Type, bool ReadOnly, const string &Source="", const string &SourceType="");
	bool sourceExists(const string &source);
	void setBySource(string source, string sourceType, string Value);
	void set(string Name, string Value);
	void set(string Name, float Value);
	float getF(string Name);
	string getS(string Name);
	const CWBControl* getControl(string Name);
	void createDeviceValues(string_map &);
	void updateValues(string_map &);
	const CControlMap *getControls(){return &m_Controls;};
	string getTopic(string Control);
	void subscribeToEntich(string_vector &v);
	void enrichDevice(const string &meta, const string &val);
	void enrichControl(const string &control, const string &meta, const string &val);
	bool isLoaded();
};

typedef map<string, CWBDevice*> CWBDeviceMap;
