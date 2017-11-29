#pragma once

#include <evntrace.h>
#include <string>


class Etw_kernel_control {
public:
	Etw_kernel_control();

private:
	PEVENT_TRACE_PROPERTIES p2session_properties = NULL;
	TRACEHANDLE session_handle = NULL;
	TRACEHANDLE trace_handle = NULL;
	EVENT_TRACE_LOGFILE trace_logfile;
};
