#include "catch_dump.h"
#include <windows.h>
#include <string>
#include <vector>
#include <Psapi.h>
#include <dbghelp.h>
#pragma comment(lib, "Dbghelp.lib")
#pragma comment(lib, "Psapi.lib")

namespace dump
{

LPTOP_LEVEL_EXCEPTION_FILTER prev_filter_ = NULL;


/*
* 功    能：枚举目录中的指定后缀的文件
* 参    数：directory：保存Dump文件目录
*           postfix：  指定后缀
*           file_list：枚举出的文件
* 返 回 值：若函数成功，则返回true
*/
bool EnumPostfixFile(const char* directory, const char* postfix, std::vector<std::string>& file_list)
{
    if (NULL == directory || NULL == postfix)
        return false;

    char temp_path[MAX_PATH*2] = {0};
    sprintf_s(temp_path, sizeof(temp_path), "%s\\*", directory);

    WIN32_FIND_DATAA file_data;
    HANDLE find_file = ::FindFirstFileA(temp_path, &file_data);
    if (INVALID_HANDLE_VALUE == find_file)
    {
        return false;
    }

    do
    {
        char* p = file_data.cFileName;
        if (p[0] == '.' && (p[1] == 0 || (p[1] == '.' && p[2] == 0)))
        {
            continue;
        }

        if (file_data.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY || file_data.dwFileAttributes == FILE_ATTRIBUTE_SYSTEM 
            || file_data.dwFileAttributes == FILE_ATTRIBUTE_VIRTUAL || file_data.dwFileAttributes == FILE_ATTRIBUTE_TEMPORARY)
        {
            continue;
        }

        sprintf_s(temp_path, sizeof(temp_path), "%s\\%s", directory, file_data.cFileName);

        if (0 == _strnicmp(temp_path + strlen(temp_path) - strlen(postfix), postfix, strlen(postfix)))
        {
            file_list.push_back(temp_path);
        }
    }while (::FindNextFileA(find_file, &file_data));

    ::FindClose(find_file);

    return true;
}

/*
* 功    能：创建Dump文件
* 参    数：file：创建Dump文件的路径
*           except_info：异常信息
* 返 回 值：函数成功，则返回true
*/
bool CreateMiniDumpFile(const char* file, PEXCEPTION_POINTERS except_info)
{
    PROCESS_MEMORY_COUNTERS pmc = {0};
    pmc.cb = sizeof(PROCESS_MEMORY_COUNTERS);
    GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc));

    HANDLE hFile = CreateFileA(file, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (NULL != hFile)
    {
        MINIDUMP_EXCEPTION_INFORMATION eInfo;
        eInfo.ThreadId = GetCurrentThreadId();
        eInfo.ExceptionPointers = except_info;
        eInfo.ClientPointers = FALSE;

        MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpWithFullMemory, except_info ? &eInfo : NULL, NULL, NULL);

        CloseHandle(hFile);
        return true;
    }

    return false;
}

/*
* 功    能：清理老的Dump文件
* 参    数：directory：保存Dump文件目录
* 返 回 值：
*/
void ClearOldDump(const char* directory)
{
    std::vector<std::string> file_list;
    if (!EnumPostfixFile(directory, ".dmp", file_list))
    {
        return;
    }

    // 如果数量超过或则等于2个，则删除最老的一个
    size_t size = file_list.size();
    if (size >= 2)
    {
        std::string del_file = file_list.at(0);
        for (size_t i = 1; i < size; i++)
        {
            if (del_file.compare(file_list.at(i)) > 0)
            {
                del_file = file_list.at(i);
            }	
        }

        ::DeleteFileA(del_file.c_str());
    }
}

/*
* 功    能：捕获异常信息，并保存
* 参    数：except_info：描述当出现异常时处理器的异常记录
* 返 回 值：回调结果
*/
LONG OnCatchException(PEXCEPTION_POINTERS except_info)
{
    char app_name[MAX_PATH] = {0};
    char path[MAX_PATH*2] = {0};
    GetModuleFileNameA(NULL, path, sizeof(path));
    for (int i = strlen(path); i > 0; i--)
    {
        if ('.' == path[i])
        {
            path[i] = '\0';
        }
        if ('\\' == path[i])
        {
            strcpy_s(app_name, sizeof(app_name), &path[i+1]);
            path[i] = '\0';
            strcat_s(path, sizeof(path), "\\Dump");
            CreateDirectoryA(path, NULL);
            break;
        }
    }

    // 清理老的dump文件
    ClearOldDump(path);

    SYSTEMTIME st;
    GetLocalTime(&st);

    char dump_file[MAX_PATH*2] = {0};
    sprintf_s(dump_file, sizeof(dump_file), "%s\\%s%04d-%02d-%02d-%02d-%02d-%02d.dmp", 
        path, app_name, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

    CreateMiniDumpFile(dump_file, except_info);

    return EXCEPTION_EXECUTE_HANDLER;
}

/*
* 功    能：设置异常处理函数回调
* 参    数：except_info：描述当出现异常时处理器的异常记录
* 返 回 值：回调结果
*/
LONG WINAPI UnhandledExceptFilter(PEXCEPTION_POINTERS except_info)
{
    return OnCatchException(except_info);

//     if (NULL != prev_filter_)
//     {
//         return prev_filter_(except_info);
//     }
// 
//     return EXCEPTION_CONTINUE_SEARCH;
}

// 初始化异常捕获
void init()
{
    if (NULL == prev_filter_)
    {
        prev_filter_ = SetUnhandledExceptionFilter(UnhandledExceptFilter);
    }
}

// 卸载异常捕获
void uninit()
{
    if (NULL != prev_filter_)
    {
        SetUnhandledExceptionFilter(prev_filter_);
    }
}

} // namespace dump
