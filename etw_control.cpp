#include <Windows.h>
#include <conio.h>
#include <strsafe.h>
#include <wmistr.h>
#include <evntrace.h>
#include <tdh.h>

#include <stdio.h>

#include "etw_control.h"


using namespace std;

//Microsoft-Windows-Eventlog  {D8909C24-5BE9-4502-98CA-AB7BDC24899D}
static const GUID provider_guid_eventlog = { 0xAE4BD3BE, 0xF36F, 0x45B6,{ 0x8D, 0x21, 0xBD, 0xD6, 0xFB, 0x83, 0x28, 0x53 } };
//Microsoft-Windows-Audio AE4BD3BE-F36F-45B6-8D21-BDD6FB832853
static const GUID provider_guid_audio = { 0xD8909C24, 0x5BE9, 0x4502,{ 0x98, 0xCA, 0xAB, 0x7B, 0xDC, 0x24, 0x89, 0x9D } };
//Microsoft-Windows-Kernel-File EDD08927-9CC4-4E65-B970-C2560FB5C289
static const GUID provider_guid_file = { 0xEDD08927, 0x9CC4, 0x4E65,{ 0xB9, 0x70, 0xC2, 0x56, 0x0F, 0xB5, 0xC2, 0x89 } };
GUID provider_guid = provider_guid_file;

// Enable ETW provider using EnableTraceEx2() and StartTrace()
TRACEHANDLE start_etw_session(PEVENT_TRACE_PROPERTIES p2session_properites) {

	// You use this member to specify that you want events written to a log file, a real-time consumer, or both. For a list of possible modes, https://msdn.microsoft.com/en-us/library/windows/desktop/aa364080(v=vs.85).aspx
	p2session_properites->LogFileMode = 0L 
		//| EVENT_TRACE_NO_PER_PROCESSOR_BUFFERING // Writes events that were logged on different processors to a common buffer. Using this mode can eliminate the issue of events appearing out of order when events are being published on different processors using system time.
		//| EVENT_TRACE_INDEPENDENT_SESSION_MODE // Indicates that a logging session should not be affected by EventWrite failures in other sessions. 
		//| EVENT_TRACE_SYSTEM_LOGGER_MODE // If the StartTraceProperties parameter LogFileMode includes this flag, the logger will be a system logger.
		//| EVENT_TRACE_USE_PAGED_MEMORY // This setting is recommended so that events do not use up the nonpaged memory. This mode is ignored if EVENT_TRACE_PRIVATE_LOGGER_MODE is set.
		//| EVENT_TRACE_STOP_ON_HYBRID_SHUTDOWN // This option stops logging on hybrid shutdown.
		//| EVENT_TRACE_PERSIST_ON_HYBRID_SHUTDOWN // This option continues logging on hybrid shutdown. 
		//| EVENT_TRACE_PRIVATE_LOGGER_MODE // This mode enforces that only the process that registered the provider GUID can start the logger session with that GUID.
		//| EVENT_TRACE_PRIVATE_IN_PROC
		//| EVENT_TRACE_BUFFERING_MODE // This mode writes events to a circular memory buffer. Events written beyond the total size of the buffer evict the oldest events still remaining in the buffer.
		| EVENT_TRACE_REAL_TIME_MODE // Delivers the events to consumers in real-time. Events are delivered when the buffers are flushed, not at the time the provider writes the event. Also, the event order is not guaranteed on computers with multiple processors. The real-time mode is more suitable for low-traffic, notification type events.
		;
	p2session_properites->BufferSize = ETW_SESSION_BUFFER_SIZE;
	p2session_properites->MinimumBuffers = ETW_SESSION_BUFFER_MIN_SIZE;
	p2session_properites->MaximumBuffers = ETW_SESSION_BUFFER_MAX_SIZE;

	// StartTrace()
	TRACEHANDLE session_handle;
	TDHSTATUS start_trace_status = StartTrace(
		&session_handle,
		(LPWSTR)((PCHAR)p2session_properites + p2session_properites->LoggerNameOffset),
		p2session_properites
	);

	if (ERROR_SUCCESS == start_trace_status) {
		printf("Start trace session successfully!\n");
		return session_handle;
	}
	else {
		printf("Start trace session failed! %lu. press any key to continue...\n", start_trace_status);
		return NULL;
	}
}

TDHSTATUS stop_etw_session(TRACEHANDLE session_handle, PEVENT_TRACE_PROPERTIES p2session_properites) {
	TDHSTATUS stop_etw_session_status = ControlTrace(
		session_handle,
		//(PWSTR)p2session_properites + p2session_properites->LoggerNameOffset,
		NULL,
		p2session_properites,
		EVENT_TRACE_CONTROL_STOP
	);
	return stop_etw_session_status;
}

PEVENT_TRACE_PROPERTIES allocate_session_properties(PWSTR etw_session_name, PWSTR etw_logfile_name) {
	// [WNODE.Buffersize] Total size of memory allocated, in bytes, for the event tracing session properties. The size of memory must include the room for the EVENT_TRACE_PROPERTIES structure plus the session name string and log file name string that follow the structure in memory.
	ULONG session_properites_size = sizeof(EVENT_TRACE_PROPERTIES) + (ETW_SESSION_NAME_MAX_LENGTH + ETW_LOGFILE_NAME_MAX_LENGTH) * sizeof(WCHAR);
	// {To-do} For Windows 10, version 1703 and later version, we can pass filtering to StartTrace. To do so, we need to pass in the new "EVENT_TRACE_PROPERTIES_V2". https://msdn.microsoft.com/en-us/library/windows/desktop/aa363689(v=vs.85).aspx
	PEVENT_TRACE_PROPERTIES p2session_properites = (PEVENT_TRACE_PROPERTIES)malloc(session_properites_size);
	if (NULL == p2session_properites) {
		printf("Unable to allocate %d bytes for properties structure.\n", session_properites_size);
		return NULL;
	}
	// setup session properites
	// Be sure to initialize the memory for this structure to zero before setting any members.
	ZeroMemory(p2session_properites, session_properites_size);
	p2session_properites->Wnode.BufferSize = session_properites_size;
	// For an NT Kernel Logger session, set this member to SystemTraceControlGuid. If this member is set to SystemTraceControlGuid or GlobalLoggerGuid, the logger will be a system logger. For a private logger session, set this member to the provider's GUID that you are going to enable for the session. If you start a session that is not a kernel logger or private logger session, you do not have to specify a session GUID.If you do not specify a GUID, ETW creates one for you.You need to specify a session GUID only if you want to change the default permissions associated with a specific session.For details, see the EventAccessControl function.
	// p2session_properites->Wnode.Guid;
	p2session_properites->Wnode.Flags = WNODE_FLAG_TRACED_GUID;
	p2session_properites->LoggerNameOffset = sizeof(EVENT_TRACE_PROPERTIES);
	p2session_properites->LogFileNameOffset = sizeof(EVENT_TRACE_PROPERTIES) + (ETW_SESSION_NAME_MAX_LENGTH * sizeof(WCHAR));
	if (NULL != etw_session_name) StringCchCopy((LPWSTR)((PCHAR)p2session_properites + p2session_properites->LoggerNameOffset), ETW_SESSION_NAME_MAX_LENGTH, etw_session_name);
	if (NULL != etw_logfile_name) StringCchCopy((LPWSTR)((PCHAR)p2session_properites + p2session_properites->LogFileNameOffset), ETW_LOGFILE_NAME_MAX_LENGTH, etw_logfile_name);
	return p2session_properites;
}

void free_session_properties(PEVENT_TRACE_PROPERTIES p2session_properites) {
	free(p2session_properites);
	return;
}