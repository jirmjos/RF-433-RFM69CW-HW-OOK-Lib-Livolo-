#include "stdafx.h"
#include "WebServer.h"
#include "IPListener.h"
#include "IPSupervisor.h"

CWebServer::CWebServer()
	:m_Supervisor(NULL), m_OwnSupervisor(true), m_Listener(NULL)
{
	m_webServerLog = CLog::Default();
}

CWebServer::~CWebServer()
{
	if (m_Supervisor)
	{
		m_Supervisor->RemoveClient(this, true);

		if (m_OwnSupervisor)
			m_Supervisor->Stop(10000);

	}
}

string CWebServer::urlDecode(string &SRC) {
	string ret;
	char ch;
	unsigned int i, ii;
	for (i = 0; i<SRC.length(); i++) {
		if (int(SRC[i]) == 37) {
			sscanf(SRC.substr(i + 1, 2).c_str(), "%x", &ii);
			ch = static_cast<char>(ii);
			ret += ch;
			i = i + 2;
		}
		else {
			ret += SRC[i];
		}
	}
	return (ret);
}

bool CWebServer::ParseHttpRequest(const string &Request, string &method, string &path, string_map &header, string_map &get_params, string_map &post_params)
{
	enum
	{
		START,
		METHOD = START,
		PATH,
		GET_PARAM_KEY,
		GET_PARAM_VALUE,
		PROTO,
		HEADER_KEY,
		HEADER_VALUE,
		POST_PARAM_KEY,
		POST_PARAM_VALUE,
		FINISH
	} pos;

	pos = START;
	int position = 0;
	method = path = "";
	header.clear();
	header.clear();
	header.clear();
	string key, value;
	size_t contentLength = 0;
	string postData;

	for_each_const(string, Request, i)
	{
		position++;

		switch (pos)
		{
		case METHOD:
			if (*i == '\n')
				throw CHaException(CHaException::ErrBadParam, "Bad request '%s' at pos %d, state %d", Request.c_str(), position, pos);
			else if (isalnum(*i))
			{
				method += *i;
			}
			else if (isblank(*i))
				pos = PATH;
			else
				throw CHaException(CHaException::ErrBadParam, "Bad request '%s' at pos %d, state %d", Request.c_str(), position, pos);

			break;
		case PATH:
			if (*i == '\n')
				pos = HEADER_KEY;
			else if (*i == '?')
				pos = GET_PARAM_KEY;
			else if (isalnum(*i) || ispunct(*i))
			{
				path += *i;
			}
			else if (isblank(*i))
				pos = PROTO;
			else if (*i == '\r') {}
			else
				throw CHaException(CHaException::ErrBadParam, "Bad request '%s' at pos %d, state %d", Request.c_str(), position, pos);
			break;
		case GET_PARAM_KEY:
			if (*i == '\n')
			{
				if (key.length()>0)
					get_params[key] = "";

				pos = HEADER_KEY;
				key = value = "";
			}
			else if (*i == '=')
				pos = GET_PARAM_VALUE;
			else if (isalnum(*i) || *i == '_' || *i == '.')
			{
				key += *i;
			}
			else if (isblank(*i))
			{
				get_params[key] = "";
				pos = PROTO;
				key = value = "";
			}
			else if (*i == '\r') {}
			else
				throw CHaException(CHaException::ErrBadParam, "Bad request '%s' at pos %d, state %d", Request.c_str(), position, pos);
			break;
		case GET_PARAM_VALUE:
			if (*i == '\n')
			{
				get_params[key] = urlDecode(value);
				pos = HEADER_KEY;
				key = value = "";
			}
			else if (*i == '&')
			{
				get_params[key] = urlDecode(value);
				pos = GET_PARAM_KEY;
				key = value = "";
			}
			else if (isalnum(*i))
			{
				value += *i;
			}
			else if (isblank(*i))
			{
				get_params[key] = urlDecode(value);
				pos = PROTO;
				key = value = "";
			}
			else if (*i == '\r') {}
			else
				throw CHaException(CHaException::ErrBadParam, "Bad request '%s' at pos %d, state %d", Request.c_str(), position, pos);
			break;
		case PROTO:
			if (*i == '\n')
				pos = HEADER_KEY;
			else if (*i == '\r') {}
			else if (isalnum(*i) || ispunct(*i) || *i == '/')
			{
			}
			else
				throw CHaException(CHaException::ErrBadParam, "Bad request '%s' at pos %d, state %d", Request.c_str(), position, pos);

			break;
		case HEADER_KEY:
			if (*i == '\n')
			{
				if (key.length() == 0)
					pos = POST_PARAM_KEY;
				else
					throw CHaException(CHaException::ErrBadParam, "Bad request '%s' at pos %d, state %d", Request.c_str(), position, pos);
			}
			else if (*i == ':')
				pos = HEADER_VALUE;
			else if (isalnum(*i) || ispunct(*i) || *i == '_')
			{
				key += *i;
			}
			else if (*i == '\r') {}
			else
				throw CHaException(CHaException::ErrBadParam, "Bad request '%s' at pos %d, state %d", Request.c_str(), position, pos);
			break;
		case HEADER_VALUE:
			if (*i == '\n')
			{
				if (key == "Content-Length")
					contentLength = atoi(value.c_str());

				header[key] = urlDecode(value);
				pos = HEADER_KEY;
				key = value = "";
			}
			else if (isprint(*i))
			{
				if (value.length()>0 || !isblank(*i))
					value += *i;
			}
			else if (*i == '\r') {}
			else
				throw CHaException(CHaException::ErrBadParam, "Bad request '%s' at pos %d, state %d", Request.c_str(), position, pos);
			break;
		case POST_PARAM_KEY:
			postData += *i;
			if (*i == '\n')
			{
				if (key.length()>0)
					post_params[key] = "";

				return true;
			}
			else if (*i == '=')
				pos = POST_PARAM_VALUE;
			else if (isalnum(*i) || *i == '_')
			{
				key += *i;
			}
			else if (*i == '\r') {}
			else
				throw CHaException(CHaException::ErrBadParam, "Bad request '%s' at pos %d, state %d", Request.c_str(), position, pos);
			break;

		case POST_PARAM_VALUE:
			postData += *i;
			if (*i == '\n')
			{
				post_params[key] = urlDecode(value);
				return true;
			}
			else if (*i == '&')
			{
				post_params[key] = urlDecode(value);
				pos = POST_PARAM_KEY;
				key = value = "";
			}
			else if (*i == '+')
			{
				value += ' ';
			}
			else if (isalnum(*i) || ispunct(*i))
			{
				value += *i;
			}
			else if (*i == '\r') {}
			else
				throw CHaException(CHaException::ErrBadParam, "Bad request '%s' at pos %d, state %d", Request.c_str(), position, pos);
			break;

		case FINISH:
			throw CHaException(CHaException::ErrBadParam, "Bad request '%s' at pos %d, state %d", Request.c_str(), position, pos);
		}
	}

	if (pos == POST_PARAM_VALUE && postData.length() == contentLength)
	{
		post_params[key] = urlDecode(value);
		return true;
	}

	return (pos == POST_PARAM_KEY && key.length() == 0);
}



void CWebServer::OnRecieve(CConnection* Conn)
{
	try
	{
		m_webServerLog->Printf(5, "CWebServerConnector::OnRecieve(%p)", Conn);

		char Buffer[HTTP_REQUEST_BUFFER_SIZE];
		size_t size = sizeof(Buffer) - 1;
		Conn->Recv(Buffer, size, false);
		Buffer[size] = 0;
		m_webServerLog->Printf(5, "Recieved %d bytes", size);

		if (size == 0)
		{
			m_webServerLog->Printf(5, "Web server onnection dead");
			m_ConnectionData.erase(Conn);
			Conn->SetAutoDelete(); //?
			Conn->Disconnect();
			m_Supervisor->RemoveConnection(Conn);
		}

		string Request;
		CConnectionData::iterator it = m_ConnectionData.find(Conn);

		if (it != m_ConnectionData.end())
			Request = it->second;

		Request += Buffer;

		m_webServerLog->Printf(10, "HTTP Request:%s", Request.c_str());
		string method, path;
		string_map headers, get_params, post_params;

		if (ParseHttpRequest(Request, method, path, headers, get_params, post_params))
		{

			if (method == "GET")
			{
				OnRequest(Conn, method, path, get_params);
			}
			else if (method == "POST")
			{
				OnRequest(Conn, method, path, post_params);
			}
			else
				throw CHaException(CHaException::ErrBadPacketType, "HTTP method '%s' not supported", method.c_str());
		}
		else
		{
			m_ConnectionData[Conn] = Request;
			m_webServerLog->Printf(10, "Request not completted");
		}

	}
	catch (CHaException ex)
	{
		m_webServerLog->Printf(4, "CWebServerConnector::OnRecieve runtime error. Error code %d, Message '%s'.\n", ex.GetCode(), ex.GetMessage().c_str());
		m_Supervisor->RemoveConnection(Conn);

		CConnectionData::iterator i = m_ConnectionData.find(Conn);
		if (i != m_ConnectionData.end())
			m_ConnectionData.erase(i);
	}
}

void CWebServer::OnDeleteConnection(CConnection* Conn)
{
	CConnectionData::iterator i = m_ConnectionData.find(Conn);
	if (i != m_ConnectionData.end())
		m_ConnectionData.erase(i);

	m_webServerLog->Printf(10, "CWebServer::OnDeleteConnection(%s)", Conn->GetPeerName().c_str());
}

void CWebServer::SetSupervisor(CSupervisor *supervisor)
{
	if (m_OwnSupervisor && m_Supervisor)
		delete m_Supervisor;

	if (supervisor)
	{
		m_Supervisor = supervisor;
		m_OwnSupervisor = false;
	}
	else
	{
		m_Supervisor = new CSupervisor();
		m_OwnSupervisor = true;
	}

	if (!m_Supervisor)
	{
		throw CHaException(CHaException::ErrInvalidConfig, "No Supervisor for CWebServer");
	}

	m_Supervisor->AddClient(this);

}

void CWebServer::OnRequest(CConnection* Conn, string method, string url, string_map &params)
{
	string Reply;
	Reply = "HTTP/1.1 404 Error\r";
	Reply += "Content-Type: text/html\r";
	Reply += "\r";
	Reply += "<html><body>";
	Reply += "Not found";
	Reply += "</body></html>";

	SendReply(Conn, Reply);

}

void CWebServer::SendReply(CConnection* Conn, string Reply, bool closeConnection)
{
	for (unsigned int pos = 0; pos<Reply.size(); pos += 2000)
	{
		Conn->Send(Reply.c_str() + pos, min((int)(Reply.size() - pos), 2000));
	}

	if (closeConnection)
		Conn->Disconnect();
}


void CWebServer::Listen(int Port)
{
	m_Listener = new CIPListener();
	m_Listener->Listen(Port);

	m_Supervisor->AddConnection(this, m_Listener);

	if (m_OwnSupervisor)
		m_Supervisor->Start();
}
