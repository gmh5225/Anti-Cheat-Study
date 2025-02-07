#pragma once
#include "Anti Debugger.h"
#include <TlHelp32.h>
#include <psapi.h>
#include "util.h"

namespace HYJ
{
	AntiDebugger::AntiDebugger()
	{

		HMODULE ntModule = GetModuleHandleA("ntdll.dll");
	//	HMODULE kernelModule = GetModuleHandleA("kernel32.dll");
	
		NtQueryInformationProcess = reinterpret_cast<WinAPITypeList::pNtqueryInformationProcess>(GetProcAddress(ntModule, "NtQueryInformationProcess"));
		//pIsDebuggerPresent = reinterpret_cast<WinAPITypeList::isDebuggerPresentType>(GetProcAddress(kernelModule, "IsDebuggerPresent"));
	
	}

	AntiDebugger::~AntiDebugger() {}




	bool AntiDebugger::isProcessDebugged() noexcept
	{
		return IsDebuggerPresent();
	}

	bool AntiDebugger::CheckDebugPort() noexcept
	{
		DWORD dwProcessDebugPort, dwReturned;
	
		NTSTATUS status = NtQueryInformationProcess(GetCurrentProcess(),
			ProcessDebugPort, 
			&dwProcessDebugPort,
			sizeof(DWORD),
			&dwReturned);
		
		if (status >= 0 )
		{
			if (dwProcessDebugPort == -1)
			{
				return true;
			}
		}

		return false;
	}

	bool AntiDebugger::CheckDebugFlags() noexcept
	{
		DWORD dwProcessFlags, dwReturned;
		NTSTATUS status = NtQueryInformationProcess(GetCurrentProcess(), ProcessDebugFlags, &dwProcessFlags, sizeof(DWORD),&dwReturned);
		if (status >= 0)
		{
			if (dwProcessFlags == 0)
			{
				return true;
			}
		}

		return false;
	}

	bool AntiDebugger::CheckDebugObjectHandle() noexcept
	{
		HANDLE hProcessDebugHandle;
		DWORD dwReturned;
		NTSTATUS status = NtQueryInformationProcess(GetCurrentProcess(), ProcessDebugObjectHandle, &hProcessDebugHandle, sizeof(HANDLE), &dwReturned);
		
		if (status >= 0)
		{
			if (hProcessDebugHandle != NULL)
			{
				return true;
			}
		}

		return false;
	}

	bool AntiDebugger::CheckProcessHeapFlags() noexcept
	{
		void* peb = reinterpret_cast<void*>(__readgsqword(0x60));
		DWORD ntGlobalFlag = *(DWORD*)((unsigned char*)peb + 0x68);
		
		if (ntGlobalFlag == 0x10 || ntGlobalFlag == 0x20 || ntGlobalFlag == 0x40)
		{
			return true;
		}

		return true;
	}

	bool AntiDebugger::CheckHardWareCheckPoint() noexcept
	{
		CONTEXT context = { 0, };
		
		context.ContextFlags = CONTEXT_DEBUG_REGISTERS;
		if (!GetThreadContext(GetCurrentThread(), &context))
		{
			return false;
		}

		return context.Dr0 || context.Dr1 || context.Dr2 || context.Dr3;
	}
	
	bool AntiDebugger::CheckParentIsDebugger()
	{
		HANDLE snapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		PROCESSENTRY32 pe = { 0 };
		pe.dwSize = sizeof(PROCESSENTRY32);
		DWORD pid = GetCurrentProcessId();
		if (Process32First(snapShot, &pe))
		{
			do {
				if (pe.th32ParentProcessID == pid)
				{
					break;
				}
			} while (Process32Next(snapShot, &pe));
		}
		CloseHandle(snapShot);
		HANDLE hProcess;
		hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
		if (hProcess == NULL)
		{
			CloseHandle(hProcess);
			return false;
		}

		char nameBuffer[MAX_PATH] = {0,};

		if (GetModuleBaseNameA(hProcess, NULL, nameBuffer, sizeof(nameBuffer)) == 0)
		{
			CloseHandle(hProcess);
			return false;
		}

		std::vector<std::string> debuggersName = { "ida.exe","ollydbg.exe","ida64.exe", "idag.exe", "idag64.exe", "idaw.exe", "idaw64.exe", "idaq.exe", "idaq64.exe", "idau.exe", "idau64.exe", "scylla.exe", "scylla_x64.exe", "scylla_x86.exe", "protection_id.exe", "x64dbg.exe", "x32dbg.exe", "windbg.exe", "reshacker.exe", "ImportREC.exe", "IMMUNITYDEBUGGER.EXE", "devenv.exe" };

		if (Util::MultiStringSearch(debuggersName, nameBuffer) == false)
		{
			return false;
		}

		return true;

	}

	bool AntiDebugger::CheckIsDebuggerPresentModified() noexcept
	{

		


		return true;

	}


	/*
	bool __declspec(naked) AntiDebugger::CheckDebuggerWithINT3()
	{

		__try
		{

			__asm int 3;
			return true;
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			return false;
		}


	}
	*/




}