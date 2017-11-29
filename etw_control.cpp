#include <Windows.h>
#include <conio.h>
#include <strsafe.h>
#include <wmistr.h>

#define INITGUID  // Include this #define to use SystemTraceControlGuid in Evntrace.h.
#include <evntrace.h>
#include <tdh.h>

#include <stdio.h>
#include <string>
#include <time.h>

#include "etw_control.h"
#include "trace_parser.h"
#include "manifestbased_provider_guid.h"


using namespace std;


// ETW初始化配置大概分为以下几步(需要注意的是每一步的配置)：
// 1. 配置ETW Session - configure_etw_session() - StartTrace()
// 2. 配置Consumer - configure_etw_consumer() - OpenTrace()
// 3. 配置Provider - configure_etw_provider() - EnableTraceEx2()
// 4. 开始 - ProcessTrace()
Etw_control::Etw_control() {
	time_t system_time = time(0);
	TDHSTATUS start_etw_status = configure_etw_session(ETW_SESSION_NAME, ETW_LOGFILE_NAME); //  + wstring(ctime(&system_time))
	if (ERROR_SUCCESS != start_etw_status) {
		wprintf(L"start_etw_status() failed with %lu\n", start_etw_status);
	}
	else
		wprintf(L"start_etw_status() successfully\n");

	// 注意这里parser_event是回调函数，可能比较容易找不到回调函数的位置。
	TRACEHANDLE handle = configure_etw_consumer((PEVENT_RECORD_CALLBACK)(Trace_parser::parser_event), NULL);
	if (INVALID_PROCESSTRACE_HANDLE == handle) {
		wprintf(L"configure_etw_consumer() failed with %lu!\n", GetLastError());
	}
	else
		wprintf(L"configure_etw_consumer() successfully!\n");

	TDHSTATUS provider_status = configure_etw_provider(&GUID(Microsoft_Windows_USB_USBPORT));
	if (ERROR_SUCCESS != provider_status) {
		wprintf(L"configure_etw_provider() failed with %lu!\n", provider_status);
	}
	else
		wprintf(L"configure_etw_provider() successfully!\n");

	TDHSTATUS process_status = ProcessTrace(&trace_handle, 1, 0, 0);
	if (ERROR_SUCCESS == process_status) {
		wprintf(L"ProcessTrace() failed with %lu!\n", process_status);
	}
	else
		wprintf(L"ProcessTrace() successfully!\n");
}


Etw_control::~Etw_control() {
	TDHSTATUS stop_etw_status = stop_etw_session();
	if (ERROR_SUCCESS != stop_etw_status) {
		wprintf(L"stop_etw_status() failed with %lu\n", stop_etw_status);
	}
	else
		wprintf(L"stop_etw_status() successfully\n");

	TDHSTATUS close_status = CloseTrace(trace_handle);
	if (ERROR_SUCCESS == close_status) {
		wprintf(L"CloseTrace() failed with %lu!\n", close_status);
	}
	else
		wprintf(L"CloseTrace() successfully!\n");
}

TDHSTATUS Etw_control::configure_etw_session(PWSTR etw_session_name, PWSTR etw_logfile_name) {
	// [WNODE.Buffersize] Total size of memory allocated, in bytes, for the event tracing session properties. The size of memory must include the room for the EVENT_TRACE_PROPERTIES structure plus the session name string and log file name string that follow the structure in memory.
	ULONG session_properites_size = sizeof(EVENT_TRACE_PROPERTIES) + (ETW_SESSION_NAME_MAX_LENGTH + ETW_LOGFILE_NAME_MAX_LENGTH) * sizeof(WCHAR);
	// {To-do} For Windows 10, version 1703 and later version, we can pass filtering to StartTrace. To do so, we need to pass in the new "EVENT_TRACE_PROPERTIES_V2". https://msdn.microsoft.com/en-us/library/windows/desktop/aa363689(v=vs.85).aspx
	p2session_properties = (PEVENT_TRACE_PROPERTIES)malloc(session_properites_size);
	if (NULL == p2session_properties) {
		printf("Unable to allocate %d bytes for properties structure.\n", session_properites_size);
		return ERROR_OUTOFMEMORY;
	}
	// setup session properties
	// Be sure to initialize the memory for this structure to zero before setting any members.
	ZeroMemory(p2session_properties, session_properites_size);
	p2session_properties->Wnode.BufferSize = session_properites_size;
	// For an NT Kernel Logger session, set this member to SystemTraceControlGuid. If this member is set to SystemTraceControlGuid or GlobalLoggerGuid, the logger will be a system logger. For a private logger session, set this member to the provider's GUID that you are going to enable for the session. If you start a session that is not a kernel logger or private logger session, you do not have to specify a session GUID.If you do not specify a GUID, ETW creates one for you.You need to specify a session GUID only if you want to change the default permissions associated with a specific session.For details, see the EventAccessControl function.
	// p2session_properites->Wnode.Guid;
	p2session_properties->Wnode.Flags = WNODE_FLAG_TRACED_GUID;
	p2session_properties->LoggerNameOffset = sizeof(EVENT_TRACE_PROPERTIES);
	p2session_properties->LogFileNameOffset = sizeof(EVENT_TRACE_PROPERTIES) + (ETW_SESSION_NAME_MAX_LENGTH * sizeof(WCHAR));
	if (NULL != etw_session_name) StringCchCopy((LPWSTR)((PCHAR)p2session_properties + p2session_properties->LoggerNameOffset), ETW_SESSION_NAME_MAX_LENGTH, etw_session_name);
	if (NULL != etw_logfile_name) StringCchCopy((LPWSTR)((PCHAR)p2session_properties + p2session_properties->LogFileNameOffset), ETW_LOGFILE_NAME_MAX_LENGTH, etw_logfile_name);
	// You use this member to specify that you want events written to a log file, a real-time consumer, or both. For a list of possible modes, https://msdn.microsoft.com/en-us/library/windows/desktop/aa364080(v=vs.85).aspx
	p2session_properties->LogFileMode = 0L
		//| EVENT_TRACE_NO_PER_PROCESSOR_BUFFERING // Writes events that were logged on different processors to a common buffer. Using this mode can eliminate the issue of events appearing out of order when events are being published on different processors using system time.
		| EVENT_TRACE_INDEPENDENT_SESSION_MODE // Indicates that a logging session should not be affected by EventWrite failures in other sessions. 
		//| EVENT_TRACE_SYSTEM_LOGGER_MODE // If the StartTraceProperties parameter LogFileMode includes this flag, the logger will be a system logger.
		//| EVENT_TRACE_USE_PAGED_MEMORY // This setting is recommended so that events do not use up the non paged memory. This mode is ignored if EVENT_TRACE_PRIVATE_LOGGER_MODE is set.
		//| EVENT_TRACE_STOP_ON_HYBRID_SHUTDOWN // This option stops logging on hybrid shutdown.
		//| EVENT_TRACE_PERSIST_ON_HYBRID_SHUTDOWN // This option continues logging on hybrid shutdown. 
		//| EVENT_TRACE_PRIVATE_LOGGER_MODE // This mode enforces that only the process that registered the provider GUID can start the logger session with that GUID.
		//| EVENT_TRACE_PRIVATE_IN_PROC
		//| EVENT_TRACE_BUFFERING_MODE // This mode writes events to a circular memory buffer. Events written beyond the total size of the buffer evict the oldest events still remaining in the buffer.
		| EVENT_TRACE_REAL_TIME_MODE // Delivers the events to consumers in real-time. Events are delivered when the buffers are flushed, not at the time the provider writes the event. Also, the event order is not guaranteed on computers with multiple processors. The real-time mode is more suitable for low-traffic, notification type events.
		;
	p2session_properties->BufferSize = ETW_SESSION_BUFFER_SIZE;
	p2session_properties->MinimumBuffers = ETW_SESSION_BUFFER_MIN_SIZE;
	p2session_properties->MaximumBuffers = ETW_SESSION_BUFFER_MAX_SIZE;

	// StartTrace()
	TDHSTATUS start_trace_status = StartTrace(
		&session_handle,
		(LPWSTR)((PCHAR)p2session_properties + p2session_properties->LoggerNameOffset),
		p2session_properties
	);

	return start_trace_status;
}

TDHSTATUS Etw_control::stop_etw_session() {
	TDHSTATUS stop_etw_session_status = ControlTrace(
		session_handle,
		//(PWSTR)p2session_properites + p2session_properites->LoggerNameOffset,
		NULL,
		p2session_properties,
		EVENT_TRACE_CONTROL_STOP
	);
	free(p2session_properties);
	return stop_etw_session_status;
}

// Enable ETW providers for sessions & Enable filtering
TDHSTATUS Etw_control::configure_etw_provider(LPCGUID provider_guid) {
	ENABLE_TRACE_PARAMETERS enable_parameters;
	ZeroMemory(&enable_parameters, sizeof(ENABLE_TRACE_PARAMETERS));
	enable_parameters.Version = ENABLE_TRACE_PARAMETERS_VERSION_2;
	enable_parameters.EnableProperty = 0L
		| EVENT_ENABLE_PROPERTY_PROCESS_START_KEY // The Process Start Key is a sequence number that identifies the process.
		| EVENT_ENABLE_PROPERTY_EVENT_KEY // event instance that will be constant across multiple trace sessions listening to this event
		//| EVENT_ENABLE_PROPERTY_SID // Include in the extended data the security identifier (SID) of the user.
		| EVENT_ENABLE_PROPERTY_STACK_TRACE // If you set EVENT_ENABLE_PROPERTY_STACK_TRACE, ETW will drop the event if the total event size exceeds 64K. If the provider is logging events close in size to 64K maximum, it is possible that enabling stack capture will cause the event to be lost. If the stack is longer than the maximum number of frames(192), the frames will be cut from the bottom of the stack.
		;
	enable_parameters.ControlFlags = 0;
	// enable_parameters.SourceId; dont know how to use?
	enable_parameters.EnableFilterDesc  = NULL;
	enable_parameters.FilterDescCount = 0;

//	Event Tracing for Windows (ETW) supports several categories of filtering.
//		Schematized filtering - This is the traditional filtering setup also called provider-side filtering. The controller defines a custom set of filters as a binary object that is passed to the provider in the EnableTrace call. It is incumbent on the controller and provider to define and interpret these filters and the controller should only log applicable events. This requires a close coupling of the controller and provider since the type and format of the binary object of what can be filtered is not defined. The TdhEnumerateProviderFilters function can be used to retrieve the filters defined in a manifest.
//		Scope filtering - Certain providers are enabled or not enabled to a session based on whether or not they meet the criteria specified by the scope filters. There are several types of scope filters that allow filtering based on the event ID, the process ID (PID), executable filename, the app ID, and the app package name. This feature is supported on Windows 8.1,Windows Server 2012 R2, and later.
//		Stackwalk filtering - This notifies ETW to only perform a stack walk for a given set of event IDs. This feature is supported on Windows 8.1,Windows Server 2012 R2, and later.
//		Event payload filtering - For manifest providers, events can be filtered on-the-fly based on whether or not they satisfy a logical expression based on one or more predicates.
// https://msdn.microsoft.com/en-us/library/windows/desktop/dd392305(v=vs.85).aspx
	TDHSTATUS enable_provider_status = EnableTraceEx2(
		session_handle,
		provider_guid,
		EVENT_CONTROL_CODE_ENABLE_PROVIDER,
		TRACE_LEVEL_VERBOSE,
		0,
		0,
		0,
		&enable_parameters);
	return enable_provider_status;
}

// Open trace session for consumer
// After calling OpenTrace, call the ProcessTrace function to process the events. When you have finished processing events, call the CloseTrace function.
TRACEHANDLE Etw_control::configure_etw_consumer(PEVENT_RECORD_CALLBACK call_back, PEVENT_TRACE_BUFFER_CALLBACK buffer_call_back) {
	ZeroMemory(&trace_logfile, sizeof(EVENT_TRACE_LOGFILE));
	trace_logfile.LoggerName = ETW_SESSION_NAME;
	trace_logfile.ProcessTraceMode = 0
		| PROCESS_TRACE_MODE_EVENT_RECORD // Specify this mode if you want to receive events in the new EVENT_RECORD format. To receive events in the new format you must specify a callback in the EventRecordCallback member. If you do not specify this mode, you receive events in the old format through the callback specified in the EventCallback member.
		| PROCESS_TRACE_MODE_RAW_TIMESTAMP // Specify this mode if you do not want the time stamp value in the TimeStamp member of EVENT_HEADER and EVENT_TRACE_HEADER converted to system time
		| PROCESS_TRACE_MODE_REAL_TIME
		;
	trace_logfile.BufferCallback = buffer_call_back;
	trace_logfile.EventRecordCallback = call_back;

	trace_handle = OpenTrace(&trace_logfile);
	return trace_handle;
}
