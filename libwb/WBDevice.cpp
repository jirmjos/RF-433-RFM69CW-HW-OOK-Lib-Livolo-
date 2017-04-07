#include "stdafx.h"
#include "WBDevice.h"
#ifdef USE_CONFIG
	#include "../libutils/ConfigItem.h"
#endif

const char *g_Topics[] =
{
	"error",
	"switch",
	"alarm",
	"pushbutton",
	"range",
	"rgb",
	"text",
	"value",
	"temperature",
	"rel_humidity",
	"atmospheric_pressure",
	"sound_level",
	"PrecipitationRate", //(rainfall rate)	rainfall	mm per hour	float
	"WindSpeed", //	wind_speed	m / s	float
	"PowerPower", //	watt	float
	"PowerConsumption", //	power_consumption	kWh	float
	"voltage", //	volts	float
	"water_flow", //	water_flow	m ^ 3 / hour	float
	"WaterTotal", // consumption	water_consumption	m ^ 3	float
	"resistance", //	resistance	Ohm	float
	"concentration", //	concentration	ppm	float(unsigned)
	"lux",
	"",
};

CWBControl::CWBControl(const string &name)
	:Name(name), fValue(0), Readonly(false), Changed(false), Type(Error), Max(100), LastError(0)
{

}

void CWBControl::enrich(const string &meta, const string &val)
{
	if (meta == "type")
	{
		setType(val);
	}
	else if (meta == "max")
	{
		Max = atoi(val);
	}
	else if (meta == "readonly")
	{
		Readonly = atoi(val) != 0;
	}
	else if (meta == "order")
	{
	}
	else if (meta == "error")
	{
		time(&LastError);
	}
	else
		throw CHaException(CHaException::ErrBadParam, "Unknown device meta '%s'", meta.c_str());
}

CWBControl::ControlType CWBControl::getType(const string& type)
{
	ControlType Type = CWBControl::Error;
	for (int i = 0; g_Topics[i][0]; i++)
	{
		if (type == g_Topics[i])
		{
			Type = (CWBControl::ControlType)i;
			break;
		}
	}

	return Type;
}

string CWBControl::getTypeName(ControlType type)
{
	if (sizeof(g_Topics) / sizeof(g_Topics[0]) > (int)type)
		return g_Topics[(int)type];
	else
		return g_Topics[0];
}

CWBControl::ControlType CWBControl::getType()
{
	return Type;
}

string CWBControl::getTypeName()
{
	return getTypeName(Type);
}

void CWBControl::setType(const string& type)
{
	Type = getType(type);

	if (Type == Error && type != "" && type != "Error")
		throw CHaException(CHaException::ErrBadParam, "Unknown device type '%s'", type.c_str());
}

bool CWBControl::isLoaded()
{
	return Type != Error;
}


CWBDevice::CWBDevice(string Name, string Description)
:m_Name(Name), m_Description(Description)
{

}

CWBDevice::CWBDevice()
{

}


CWBDevice::~CWBDevice()
{
	for_each(CControlMap, m_Controls, i)
		delete i->second;
}

#ifdef USE_CONFIG
void CWBDevice::Init(CConfigItem config)
{
	m_Name = config.getStr("Name");
	m_Description = config.getStr("Description");

	CConfigItemList controls;
	config.getList("Control", controls);
	for_each(CConfigItemList, controls, control)
	{
		CWBControl *Control = new CWBControl((*control)->getStr("Name"));
		Control->Source = (*control)->getStr("Source", false);
		Control->SourceType = (*control)->getStr("SourceType", false);
		Control->Readonly = (*control)->getInt("Readonly", false, 1) != 0;
		Control->Max = (*control)->getInt("Max", false, 100);
		Control->setType((*control)->getStr("Type"));
		m_Controls[Control->Name] = Control;
	}
}
#endif

void CWBDevice::addControl(const string &Name)
{
	if (m_Controls.find(Name) != m_Controls.end())
		return;

	CWBControl *Control = new CWBControl(Name);
	Control->Name = Name;
	m_Controls[Control->Name] = Control;
}

void CWBDevice::addControl(const string &Name, CWBControl::ControlType Type, bool ReadOnly, const string &Source, const string &SourceType)
{
	CWBControl *Control = new CWBControl(Name);
	Control->Source = Source;
	Control->SourceType = SourceType;
	Control->Readonly = ReadOnly;
	Control->Type = Type;
	m_Controls[Control->Name] = Control;
}


void CWBDevice::set(string Name, string Value)
{
	CControlMap::iterator i = m_Controls.find(Name);

	if (i == m_Controls.end())
		throw CHaException(CHaException::ErrBadParam, Name);

	i->second->fValue = (float)atof(Value);
	i->second->sValue = Value;
	i->second->Changed = true;
	i->second->LastError = 0;
}

void CWBDevice::set(string Name, float Value)
{
	CControlMap::iterator i = m_Controls.find(Name);

	if (i == m_Controls.end())
		throw CHaException(CHaException::ErrBadParam, Name);

	i->second->fValue = Value;
	i->second->sValue = ftoa(Value);
	i->second->Changed = true;
}


float CWBDevice::getF(string Name)
{
	CControlMap::iterator i = m_Controls.find(Name);

	if (i == m_Controls.end())
		throw CHaException(CHaException::ErrBadParam, Name);

	return i->second->fValue;
}

int CWBDevice::getI(string Name)
{
	CControlMap::iterator i = m_Controls.find(Name);

	if (i == m_Controls.end())
		throw CHaException(CHaException::ErrBadParam, Name);

	return atoi(i->second->sValue);
}

string CWBDevice::getS(string Name)
{
	CControlMap::iterator i = m_Controls.find(Name);

	if (i == m_Controls.end())
		throw CHaException(CHaException::ErrBadParam, Name);

	return i->second->sValue;
}

const CWBControl* CWBDevice::getControl(string Name)
{
	CControlMap::iterator i = m_Controls.find(Name);

	if (i == m_Controls.end())
		throw CHaException(CHaException::ErrBadParam, Name);

	return i->second;
}

void CWBDevice::createDeviceValues(string_map &v)
{
	string base = "/devices/" + m_Name;
	v[base + "/meta/name"] = m_Description;

	for_each(CControlMap, m_Controls, i)
	{
		v[base + "/meta/name"] = m_Description;
		v[base + "/controls/" + i->first] = i->second->sValue;
		v[base + "/controls/" + i->first +"/meta/type"] = g_Topics[i->second->Type];
		if (i->second->Readonly)
			v[base + "/controls/" + i->first + "/meta/readonly"] = "1";
	}
	//UpdateValues(v);
}

void CWBDevice::updateValues(string_map &v)
{
	string base = "/devices/" + m_Name;

	for_each(CControlMap, m_Controls, i)
	{
		if (i->second->Changed)
		{
			v[base + "/controls/" + i->first] = i->second->sValue;
			i->second->Changed = false;
		}
	}
}

string CWBDevice::getTopic(string Control)
{
	string base = "/devices/" + m_Name;
	return base + "/controls/" + Control;
}

bool CWBDevice::sourceExists(const string &source)
{
	for_each(CControlMap, m_Controls, i)
	{
		if (i->second->Source == source)
			return true;
	}

	return false;
}

bool CWBDevice::controlExists(string Name)
{
	CControlMap::iterator i = m_Controls.find(Name);

	if (i == m_Controls.end())
		return false;

	return true;
}

void CWBDevice::setBySource(string source, string sourceType, string Value)
{
	if (sourceType=="X10")
		Value = (Value==("ON"?"1":"0"));

	for_each(CControlMap, m_Controls, i)
	{
		if (i->second->Source == source)
			set(i->first, Value);
	}
}

void CWBDevice::subscribeToEnrich(string_vector &v)
{
	if (m_Description.size()==0)
		v.push_back("/devices/" + m_Name+ "/meta/#");

	for_each(CControlMap, m_Controls, i)
	{
		if (!i->second->isLoaded())
			v.push_back(getTopic(i->second->Name)+"/meta/#");
	}
}

void CWBDevice::enrichDevice(const string &meta, const string &val)
{
	if (meta == "name")
	{
		m_Description = val;
	}
	else
		throw CHaException(CHaException::ErrBadParam, "Unknown device meta '%s'", meta.c_str());
}

void CWBDevice::enrichControl(const string &control, const string &meta, const string &val)
{
	if (m_Controls.find(control) == m_Controls.end())
		addControl(control);

	m_Controls[control]->enrich(meta, val);
}

bool CWBDevice::isLoaded()
{
	if (m_Description.size() == 0)
		return false;

	for_each(CControlMap, m_Controls, i)
	{
		if (!i->second->isLoaded())
			return false;
	}

	return true;
}