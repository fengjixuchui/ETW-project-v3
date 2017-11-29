#pragma once

#include <evntrace.h>
#include <string>


#define ETW_SESSION_NAME_MAX_LENGTH 20 
#define ETW_SESSION_NAME L"darpa_detection" // the length of ETW_SESSION_NAME should not larger than ETW_SESSION_NAME_MAX_LENGTH
#define ETW_LOGFILE_NAME_MAX_LENGTH 100
#define ETW_LOGFILE_NAME L"E:\\Programming\\ETW_project\\User level providers\\logfile\\demo.etl"

#define ETW_SESSION_BUFFER_SIZE 512
#define ETW_SESSION_BUFFER_MAX_SIZE 64
#define ETW_SESSION_BUFFER_MIN_SIZE 8


class Etw_control {
public:
	GUID provider_guid_file = { 0xEDD08927, 0x9CC4, 0x4E65,{ 0xB9, 0x70, 0xC2, 0x56, 0x0F, 0xB5, 0xC2, 0x89 } };
	GUID provider_guid = provider_guid_file;

	Etw_control();
	Etw_control(std::wstring);

	~Etw_control();

	// Enable ETW providers for sessions & Enable filtering
	TDHSTATUS configure_etw_provider(LPCGUID provider_guid);
	// Open trace session for consumer
	TRACEHANDLE configure_etw_consumer(PEVENT_RECORD_CALLBACK call_back, PEVENT_TRACE_BUFFER_CALLBACK buffer_call_back);

private:
	PEVENT_TRACE_PROPERTIES p2session_properties = NULL;
	TRACEHANDLE session_handle = NULL;
	TRACEHANDLE trace_handle = NULL;
	EVENT_TRACE_LOGFILE trace_logfile;

	// Set up ETW sessions & Enable ETW provider
	TDHSTATUS configure_etw_session(PWSTR etw_session_name, PWSTR etw_logfile_name);
	// Stop ETW sessions
	TDHSTATUS stop_etw_session();

};
