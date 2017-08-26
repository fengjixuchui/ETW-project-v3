#pragma once

#define ETW_SESSION_NAME_MAX_LENGTH 20 
#define ETW_SESSION_NAME L"darpa_detection" // the length of ETW_SESSION_NAME should not larger than ETW_SESSION_NAME_MAX_LENGTH
#define ETW_LOGFILE_NAME_MAX_LENGTH 100
#define ETW_LOGFILE_NAME L"E:\\Programming\\ETW_project\\User level providers\\logfile\\demo.etl"

#define ETW_SESSION_BUFFER_SIZE 512
#define ETW_SESSION_BUFFER_MAX_SIZE 64
#define ETW_SESSION_BUFFER_MIN_SIZE 8

// Set up ETW sessions & Enable ETW provider
TRACEHANDLE start_etw_session(PEVENT_TRACE_PROPERTIES);
// Stop ETW sessions
TDHSTATUS stop_etw_session(TRACEHANDLE, PEVENT_TRACE_PROPERTIES);

// Allocate space for session properties & do some simple settings
PEVENT_TRACE_PROPERTIES allocate_session_properties(PWSTR, PWSTR);
// Free session properties
void free_session_properties(PEVENT_TRACE_PROPERTIES);