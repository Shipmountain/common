#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#include <string>
#include <windows.h>
#include <tlhelp32.h>
#include <map>

namespace sys
{
    static bool get_process_list(std::map<unsigned, std::wstring>& process_lst)
    {
        PROCESSENTRY32W pe32 = {0};
        pe32.dwSize = sizeof(pe32);
        HANDLE hProcessSnap=::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
        if (INVALID_HANDLE_VALUE == hProcessSnap)
        {
            return false;
        }

        BOOL bMore=::Process32FirstW(hProcessSnap,&pe32);
        while(bMore)
        {
            process_lst.insert(std::make_pair(pe32.th32ProcessID, pe32.szExeFile));
            bMore=::Process32NextW(hProcessSnap,&pe32);
        }
        ::CloseHandle(hProcessSnap);
        return true;
    }
	
	static bool get_module_path(std::string& module_path)
    {
        char module_name[MAX_PATH] = { 0 };
        if (::GetModuleFileNameA(NULL, module_name, MAX_PATH) == FALSE)
        {
            return false;
        }

        char driver[MAX_PATH] = { 0 };
        char dir[MAX_PATH] = { 0 };
        char file_name[MAX_PATH] = { 0 };
        char ext[MAX_PATH] = { 0 };

        ::_splitpath_s(module_name, driver, MAX_PATH, dir, MAX_PATH, file_name, MAX_PATH, ext, MAX_PATH);
        module_path = std::string(driver) + std::string(dir);
        return true;
    }

};


#endif //_SYSTEM_H_
