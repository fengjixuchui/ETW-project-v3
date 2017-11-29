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

#include "trace_parser.h"
#include "ntkernel_provider_guid.h"
#include "etw_kernel_control.h"


// 用这个函数来设置ETW的Kernel的Session和Logger。
// 使用kernel的ETW初始化配置不需要单独配置Provider，因此只有3步：
// 1. 配置ETW Session - configure_etw_session() - StartTrace()
// 2. 配置Consumer - configure_etw_consumer() - OpenTrace()
// 3. 开始 - ProcessTrace()
Etw_kernel_control::Etw_kernel_control() {

// ==============================set up trace session=========================== 
	ULONG status = ERROR_SUCCESS;
	TRACEHANDLE SessionHandle = 0;
	EVENT_TRACE_PROPERTIES* pSessionProperties = NULL;
	ULONG BufferSize = 0;
	BufferSize = sizeof(EVENT_TRACE_PROPERTIES)
				 // + sizeof(LOGFILE_PATH) // delete this in real time mode
				 +sizeof(KERNEL_LOGGER_NAME);
	pSessionProperties = (EVENT_TRACE_PROPERTIES*)malloc(BufferSize);
	if (NULL == pSessionProperties) {
		wprintf(L"Unable to allocate %d bytes for properties structure.\n", BufferSize);
	}

	// Set the session properties. You only append the log file name
	// to the properties structure; the StartTrace function appends
	// the session name for you.
	ZeroMemory(pSessionProperties, BufferSize);
	pSessionProperties->Wnode.BufferSize = BufferSize;
	pSessionProperties->Wnode.Flags = WNODE_FLAG_TRACED_GUID;
	pSessionProperties->Wnode.ClientContext = 1; //QPC clock resolution
	pSessionProperties->Wnode.Guid = SystemTraceControlGuid;
	pSessionProperties->EnableFlags = 0L // https://msdn.microsoft.com/en-us/library/windows/desktop/aa363784(v=vs.85).aspx
		//| EVENT_TRACE_FLAG_SYSTEMCALL
		//| EVENT_TRACE_FLAG_CSWITCH  
		//| EVENT_TRACE_FLAG_REGISTRY
		//| EVENT_TRACE_FLAG_IMAGE_LOAD
		//| EVENT_TRACE_FLAG_FILE_IO_INIT
		//| EVENT_TRACE_FLAG_NETWORK_TCPIP
		//| EVENT_TRACE_FLAG_DISK_FILE_IO
		//| EVENT_TRACE_FLAG_FILE_IO	
		| EVENT_TRACE_FLAG_PROCESS					// process start & end
		//| EVENT_TRACE_FLAG_THREAD						// thread start & end
		//| EVENT_TRACE_FLAG_IMAGE_LOAD					// image load
		//| EVENT_TRACE_FLAG_FILE_IO					// file IO
		//| EVENT_TRACE_FLAG_DISK_FILE_IO				// requires disk IO
		//| EVENT_TRACE_FLAG_REGISTRY					// registry calls
		//| EVENT_TRACE_FLAG_CSWITCH					// context switches
		//| EVENT_TRACE_FLAG_SYSTEMCALL					// system calls
		//| EVENT_TRACE_FLAG_ALPC						// ALPC traces
		//| EVENT_TRACE_FLAG_DISK_IO					// physical disk IO
		//| EVENT_TRACE_FLAG_MEMORY_PAGE_FAULTS			// all page faults
		//| EVENT_TRACE_FLAG_MEMORY_HARD_FAULTS			// hard faults only
		//| EVENT_TRACE_FLAG_NETWORK_TCPIP				// tcpip send & receive
		//| EVENT_TRACE_FLAG_DBGPRINT					// DbgPrint(ex) Calls
		//| EVENT_TRACE_FLAG_PROCESS_COUNTERS			// process perf counters
		//| EVENT_TRACE_FLAG_DPC						// deffered procedure calls
		//| EVENT_TRACE_FLAG_INTERRUPT					// interrupts
		//| EVENT_TRACE_FLAG_DISK_IO_INIT				// physical disk IO initiation
		//| EVENT_TRACE_FLAG_SPLIT_IO					// split io traces (VolumeManager)
		//| EVENT_TRACE_FLAG_DRIVER						// driver delays
		//| EVENT_TRACE_FLAG_PROFILE					// sample based profiling
		//| EVENT_TRACE_FLAG_FILE_IO_INIT				// file IO initiation     
		//| EVENT_TRACE_FLAG_DISPATCHER					// scheduler (ReadyThread)
		//| EVENT_TRACE_FLAG_VIRTUAL_ALLOC				// VM operations
		//| EVENT_TRACE_FLAG_PROFILE
		;

	//pSessionProperties->LogFileMode = EVENT_TRACE_FILE_MODE_CIRCULAR;
	//pSessionProperties->MaximumFileSize = 1;  //  MB


	pSessionProperties->LogFileMode = EVENT_TRACE_REAL_TIME_MODE;
	pSessionProperties->MaximumBuffers = 200;
	pSessionProperties->BufferSize = 1000;
	pSessionProperties->LogFileNameOffset = 0;



	//pSessionProperties->LoggerNameOffset = sizeof(EVENT_TRACE_PROPERTIES);
	//pSessionProperties->LogFileNameOffset = sizeof(EVENT_TRACE_PROPERTIES) + sizeof(KERNEL_LOGGER_NAME); 
	//StringCbCopy((LPWSTR)((char*)pSessionProperties + pSessionProperties->LogFileNameOffset), sizeof(LOGFILE_PATH), LOGFILE_PATH);

	// Create the trace session.

	status = StartTrace((PTRACEHANDLE)&SessionHandle, KERNEL_LOGGER_NAME, pSessionProperties);

	if (ERROR_SUCCESS != status){
		if (ERROR_ALREADY_EXISTS == status){
			wprintf(L"The NT Kernel Logger session is already in use.\n");
		}
		else{
			wprintf(L"EnableTrace() failed with %lu\n", status);
		}
	}

	session_handle = SessionHandle;
	p2session_properties = pSessionProperties;
// ==========================setup stackwalk ========================
// https://msdn.microsoft.com/en-us/library/windows/desktop/dd392328(v=vs.85).aspx
// Call this function after calling StartTrace.
//If the InformationClass parameter is set to TraceStackTracingInfo, calling this function enables stack tracing of the specified kernel events. Subsequent calls to this function overwrites the previous list of kernel events for which stack tracing is enabled. To disable stack tracing, call this function with InformationClass set to TraceStackTracingInfo and InformationLength set to 0.
//The extended data section of the event will include the call stack. The StackWalk_Event MOF class defines the layout of the extended data.
//Typically, on 64-bit computers, you cannot capture the kernel stack in certain contexts when page faults are not allowed. To enable walking the kernel stack on x64, set the DisablePagingExecutive Memory Management registry value to 1. The DisablePagingExecutive registry value is located under the following registry key:
//HKEY_LOCAL_MACHINE\System\CurrentControlSet\Control\Session Manager\Memory Management
//You should consider the cost of setting this registry value before doing so.

	// 其他人使用的例子：https://famellee.wordpress.com/2012/08/
	// kernel的所有event：https://docs.google.com/spreadsheets/d/1HoAByt6bAIw5YM7RRgKV41hFg9aEC_oUisTkz5IzHn0/edit?usp=sharing
	TRACE_INFO_CLASS informationclass = TraceStackTracingInfo;
	CLASSIC_EVENT_ID enable_stack_provider_class[] = { 
		//{ProcessGuid, 3, {0}},
		{ProcessGuid, 1, {0}},
		//{ProcessGuid, 2, {0}},
		//{ProcessGuid, 11, {0}}
	}; // https://msdn.microsoft.com/en-us/library/windows/desktop/dd392304(v=vs.85).aspx
	TraceSetInformation(session_handle, informationclass, enable_stack_provider_class, sizeof(enable_stack_provider_class)); 
	//configure_etw_provider(&GUID({ 0x9a280ac0,0xc8e0,0x11d1,{0x84, 0xe2, 0x00, 0xc0, 0x4f, 0xb9, 0x98, 0xa2 } }));
	//configure_etw_provider(&GUID({ 0x3d6fa8d4,0xfe05,0x11d0,{0x9d, 0xda, 0x00, 0xc0, 0x4f, 0xd7, 0xba, 0x7c } }));
	//TDHSTATUS provider_status = configure_etw_provider(&GUID({ 0x45d8cccd, 0x539f, 0x4b72, {0xa8, 0xb7, 0x5c, 0x68, 0x31, 0x42, 0x60, 0x9a } }));

// ==========================set up call back function & start process ========================

	EVENT_TRACE_LOGFILE event_logfile;
	TRACE_LOGFILE_HEADER* event_logfile_header;
	TRACEHANDLE event_logfile_handle;
	BOOL event_usermode = FALSE;

	event_logfile_header = &event_logfile.LogfileHeader;
	ZeroMemory(&event_logfile, sizeof(EVENT_TRACE_LOGFILE));
	event_logfile.LoggerName = KERNEL_LOGGER_NAME;

	// consum_event() is the callback function. should be writed in this class.
	// If everything go well, the program will be block here.
	event_logfile.EventRecordCallback = (PEVENT_RECORD_CALLBACK)(Trace_parser::parser_kernel_event);

	event_logfile.ProcessTraceMode = PROCESS_TRACE_MODE_REAL_TIME | PROCESS_TRACE_MODE_EVENT_RECORD;
	event_logfile_handle = OpenTrace(&event_logfile);
	if (INVALID_PROCESSTRACE_HANDLE == event_logfile_handle){
		wprintf(L"OpenTrace failed with %lu\n", GetLastError());
	}

	event_usermode = event_logfile_header->LogFileMode & EVENT_TRACE_PRIVATE_LOGGER_MODE;
	if (event_logfile_header->PointerSize != sizeof(PVOID)){
		event_logfile_header = (PTRACE_LOGFILE_HEADER)((PUCHAR)event_logfile_header +
			2 * (event_logfile_header->PointerSize - sizeof(PVOID)));
	}

	TDHSTATUS temp_status = ProcessTrace(&event_logfile_handle, 1, 0, 0);
	if (temp_status != ERROR_SUCCESS && temp_status != ERROR_CANCELLED){
		wprintf(L"ProcessTrace failed with %lu\n", temp_status);
	}
}