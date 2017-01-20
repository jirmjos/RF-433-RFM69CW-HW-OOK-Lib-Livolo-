#pragma once
#include "SupervisorClient.h"
#include "IPListener.h"
#include "../libutils/strutils.h"

typedef map<CConnection*, string> CConnectionData;
#define HTTP_REQUEST_BUFFER_SIZE 4096

class LIBCOMM_API CWebServer :
	public CSupervisorClient
{
	CIPListener *m_Listener;
	CConnectionData m_ConnectionData;
	bool m_OwnSupervisor;

protected:
	CLog* m_webServerLog;
	CSupervisor* m_Supervisor;

public:
	CWebServer();
	~CWebServer();

	string urlDecode(string &SRC);
	bool ParseHttpRequest(const string &Request, string &method, string &path, string_map &header, string_map &get_params, string_map &post_params);
	inline bool ParseHttpRequest(const char *request, string &method, string &path, string_map &header, string_map &get_params, string_map &post_params)
	{
		string Request = request;
		return ParseHttpRequest(Request, method, path, header, get_params, post_params);
	}

	virtual void OnRecieve(CConnection* Conn);
	virtual void OnDeleteConnection(CConnection* Conn);
	virtual void OnRequest(CConnection* Conn, string method, string url, string_map &params);

	void SetLog(CLog *log) { m_webServerLog = log; };
	void SetSupervisor(CSupervisor *supervisor);
	void Listen(int Port);

	void SendReply(CConnection* conn, string reply, bool closeConnection = true);
};

