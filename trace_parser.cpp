#include <Windows.h>
#include <conio.h>
#include <strsafe.h>
#include <wmistr.h>
#include <evntrace.h>
#include <tdh.h>
#include <evntcons.h>

#include <stdio.h>
#include <iostream>
#include <fstream>

#include "trace_parser.h"


using namespace std;

VOID WINAPI Trace_parser::parser_event(PEVENT_RECORD Buffer) {
	void *buffer_postion = &Buffer;
	//wcout << "------------call back function get record successfully----------------" << endl;

	// ��֤�˲��� - ExtendData���������ʽ���ں����
	wcout << hex << Buffer->EventHeader.ProviderId.Data1 << " - ";
	wcout << dec << Buffer->EventHeader.EventDescriptor.Opcode << endl;
	PEVENT_HEADER_EXTENDED_DATA_ITEM ExtendedData_array = (Buffer->ExtendedData);
	PEVENT_EXTENDED_ITEM_STACK_TRACE64 PStack64;
	for (int i = 0; i < Buffer->ExtendedDataCount; i++, ExtendedData_array++) {
		//wcout << ExtendedData_array->ExtType << " " << ExtendedData_array->DataSize << " " << ExtendedData_array->Linkage << endl;

		// ExtendData��ExtTypeΪ6����64λ����ջ�Ľṹ����ͷ�ļ��ж��巽�����
		// typedef struct _EVENT_EXTENDED_ITEM_STACK_TRACE64 {
		//					ULONG64 MatchId;
		//					ULONG64 Address[ANYSIZE_ARRAY];
		//				  }
		if (ExtendedData_array->ExtType == 6) {
			PStack64 = (PEVENT_EXTENDED_ITEM_STACK_TRACE64)ExtendedData_array->DataPtr;
			wcout << hex << PStack64->MatchId << endl;
			for (int j = 0; j < (ExtendedData_array->DataSize - 4) / 4; j++){
				wcout << PStack64->Address[j] << ",";
			}
			wcout << dec << endl;
		}
	}

	// try to parse the extended data(for now, it will be call stack information)
	//int call_stack_leangth = Buffer->ExtendedData
	return;
}