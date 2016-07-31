#include "stdafx.h"
#include "libutils.h"
#include "Config.h"
#include "Exception.h"
//#include "strutils.h"


CConfig::CConfig(void)
{
}

CConfig::~CConfig(void)
{
#ifdef _LIBUTILS_USE_XML_LIBXML2
	m_Document.Close();
#endif
}

void CConfig::Close()
{
}

void CConfig::Load(string ConfigFileName)
{
#ifdef _LIBUTILS_USE_XML_LIBXML2
	//
	//  Parse the XML file, catching any XML exceptions that might propogate
	//  out of it.
	//
	m_Document.Load(ConfigFileName);
#elif defined(USE_JSON)
	Json::Reader reader;
	Json::Value root;
	ifstream file(ConfigFileName);
	bool parsingSuccessful = reader.parse(file, root);
	if (!parsingSuccessful)
		throw CHaException(CHaException::ErrParsingError, "Failed to parse %s. Error %s", ConfigFileName, reader.getFormatedErrorMessages().c_str());
	m_Document = root;
#else
#	error usupported configuration
#endif
	//

}

string CConfig::getStr(const char* path)
{
#ifdef _LIBUTILS_USE_XML_LIBXML2
	return m_Document.getStr(path);
#elif defined(USE_JSON)
	return  m_Document.GetAttribute(path);
#endif
}

CConfigItem CConfig::getNode(const char* path)
{
	return m_Document.getNode(path);
}

void CConfig::getList(const char* path, CConfigItemList &list)
{
	 m_Document.getList(path, list);
}
