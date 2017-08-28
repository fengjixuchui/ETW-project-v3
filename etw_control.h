#pragma once

#define ETW_SESSION_NAME_MAX_LENGTH 20 
#define ETW_SESSION_NAME L"darpa_detection" // the length of ETW_SESSION_NAME should not larger than ETW_SESSION_NAME_MAX_LENGTH
#define ETW_LOGFILE_NAME_MAX_LENGTH 100
#define ETW_LOGFILE_NAME L"E:\\Programming\\ETW_project\\User level providers\\logfile\\demo.etl"

#define ETW_SESSION_BUFFER_SIZE 512
#define ETW_SESSION_BUFFER_MAX_SIZE 64
#define ETW_SESSION_BUFFER_MIN_SIZE 8


class Etw_control {
public:
	Etw_control();
	~Etw_control();

	// Enable ETW providers for sessions & Enable filtering
	TDHSTATUS enable_etw_provider(TRACEHANDLE, LPCGUID);

private:
	PEVENT_TRACE_PROPERTIES p2session_properties = NULL;
	TRACEHANDLE session_handle = NULL;

	// Set up ETW sessions & Enable ETW provider
	TDHSTATUS start_etw_session(PWSTR etw_session_name, PWSTR etw_logfile_name);
	// Stop ETW sessions
	TDHSTATUS stop_etw_session();
};
