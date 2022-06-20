#include "log.h"
#include <string.h>
#include <vector>
#include <functional>
#include <stdlib.h>
#include <locale.h>

#include <time.h>
#include <sys\timeb.h> 
#pragma  warning(disable:4996)

namespace Log
{
    //////////////////////////////////////////////////////////////////////////

    const wchar_t HEADER_TIMESTAMP_W[] = L"[%02u:%02u:%02u.%03u]";
    const wchar_t HEADER_THREADID_W[] = L"|[%04x]";
    const wchar_t HEADER_STRING_W[] = L"|[%s]";
    const wchar_t TAIL_STRING_W[] = L"\r\n\0";

    const char    HEADER_TIMESTAMP_A[] = "[%02u:%02u:%02u.%03u]";
    const char    HEADER_THREADID_A[] = "|[%04x]";
    const char    HEADER_STRING_A[] = "|[%s]";
    const char    TAIL_STRING_A[] = "\r\n\0";

    const char    FILE_NAME[] = "%s\\%s_%04u%02u%02u_%d.log";
    const char    FILE_POSTFIX[] = ".log";


    const int LOG_NAME_LEN = 260;        // Ãû³Æ³¤¶È
    const int LOG_PATH_LEN = 512;        // ÈÕÖ¾Â·¾¶³¤¶È
    const int LOG_BUFFER_LEN = 1024;       // ÈÕÖ¾»º´æÄ¬ÈÏ³¤¶È
    const int LOG_TAIL_LEN = 3;          // ÈÕÖ¾½áÎ²³¤¶È
    const int MB_UNIT = 1024 * 1024;  // 1Mµ¥Î»

    const int LOG_DEF_FILE_SIZE = 1024;     // ÈÕÖ¾Ä¬ÈÏ´óÐ¡Îª1024M
    const int LOG_DEF_LIVE_DAY = 10;      // ÈÕÖ¾Ä¬ÈÏ±£´æ10Ìì
    const int LOG_DEF_TYPE = LogScreen | LogVSDebugWindow | LogDiskFile;
    const int LOG_DEF_LEVEL = LogStartup | LogShutdown | LogInfo | LogTrace | LogDebug | LogWarning | LogError | LogFatal;

    //////////////////////////////////////////////////////////////////////////

        // »ñµÃÁ½¸öÊ±¼äÖ®¼äµÄ²î±ðÌìÊý
        int GetDiffDays(const FILETIME &ftOld, struct tm &stNow)
        {
            FILETIME ftNow;
            struct tm tm_tmp = stNow;
            --tm_tmp.tm_mon;
            tm_tmp.tm_year -= 1900;
            time_t tt = mktime(&tm_tmp);
            LONGLONG ll = Int32x32To64(tt, 10000000) + 116444736000000000;
            ftNow.dwLowDateTime = (DWORD)ll;
            ftNow.dwHighDateTime = (DWORD)(ll >> 32);

            ULARGE_INTEGER uiOldTime, uiNowTime;
            uiOldTime.LowPart = ftOld.dwLowDateTime;
            uiOldTime.HighPart = ftOld.dwHighDateTime;
            uiNowTime.LowPart = ftNow.dwLowDateTime;
            uiNowTime.HighPart = ftNow.dwHighDateTime;

            LONGLONG diff_time = uiNowTime.QuadPart - uiOldTime.QuadPart;
            return int((diff_time / 10000000) / (60 * 60 * 24));
        }

    bool DelFile(const char* file)
    {
        if (NULL == file)
        {
            return false;
        }

        return (TRUE == DeleteFileA(file));
    }

    // »ñµÃ2ÊýÖ®¼äµÄ±¶Êý
    unsigned int GetMultiple(unsigned int num1, unsigned int num2)
    {
        int mult = num1 / num2;
        if (0 != (num1 % num2))
        {
            ++mult;
        }
        return mult;
    }

    // ¿í×Ö·û×ªÕ­×Ö·û
    bool WCharToMByte(const wchar_t* wide_str, char* multi_str, size_t multi_size)
    {
        size_t min_size = wcstombs(NULL, wide_str, 0);
        if (multi_size < min_size)
        {
            return false;
        }

        wcstombs(multi_str, wide_str, min_size);
        return true;
    }

    // Õ­×Ö·û×ª¿í×Ö·û
    bool MByteToWChar(const char* multi_str, wchar_t* wide_str, size_t wide_size)
    {
        size_t min_size = mbstowcs(NULL, multi_str, 0);
        if (wide_size < min_size)
        {
            return false;
        }
        mbstowcs(wide_str, multi_str, min_size);
        return true;
    }

    // »ñµÃÎÄ¼þ³¤¶È
    bool GetFileLength(FILE* fp, long& length)
    {
        if (NULL == fp)
        {
            return false;
        }

        long curr = ftell(fp);

        if (0 != fseek(fp, 0, SEEK_END))
        {
            return false;
        }

        length = ftell(fp);
        fseek(fp, curr, SEEK_SET);

        return true;
    }

    // ´´½¨ÍêÕûÄ¿Â¼
    bool CreateFullDirectory(const char* directory)
    {
        if (NULL == directory)
        {
            return false;
        }

        char* temp = NULL;
        char* start_pos = NULL;
        char path_name[512] = { 0 };
        strcpy(path_name, directory);

        for (start_pos = path_name; NULL != *start_pos && (start_pos - path_name) < (int)strlen(path_name); start_pos = temp + 1)
        {
            temp = strchr(start_pos, '\\');
            if (temp > path_name && *(temp - 1) == ':')
            {
                continue;
            }

            if (temp == path_name && *(temp + 1) == '\\')
            {
                temp = strchr(temp + 2, '\\');
                if (NULL == temp)
                {
                    break;
                }
                continue;
            }

            if (NULL != temp)
            {
                *temp = 0;
                if (!CreateDirectoryA(path_name, NULL))
                {
                    if (GetLastError() != ERROR_ALREADY_EXISTS)
                    {
                        return false;
                    }
                }
                *temp = '\\';
            }
            else
            {
                if (path_name[strlen(path_name) - 1] != ':' && !CreateDirectoryA(path_name, NULL))
                {
                    if (GetLastError() != ERROR_ALREADY_EXISTS)
                    {
                        return false;
                    }
                }
                break;
            }
        }

        return true;
    }

    // ËÑË÷Ä¿Â¼£¬²¢Ö´ÐÐ»Øµ÷
    bool SearchDirectory(const char* directory, bool is_in_subdir, std::function<bool(const char*, const WIN32_FIND_DATAA&)> filterfile)
    {
        char find_path[512] = { 0 };
        sprintf_s(find_path, sizeof(find_path), "%s\\*", directory);

        WIN32_FIND_DATAA fd = { 0 };
        HANDLE find_handle = FindFirstFileA(find_path, &fd);
        if (INVALID_HANDLE_VALUE == find_handle)
        {
            return false;
        }

        while (FindNextFileA(find_handle, &fd))
        {
            if (0 == _strcmpi(fd.cFileName, ".") || 0 == _strcmpi(fd.cFileName, ".."))
            {
                continue;
            }

            if ((0 != (fd.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)) ||
                (0 != (fd.dwFileAttributes & FILE_ATTRIBUTE_VIRTUAL)) ||
                (0 != (fd.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY)))
            {
                continue;
            }

            sprintf_s(find_path, sizeof(find_path), "%s\\%s", directory, fd.cFileName);

            if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
            {
                if (is_in_subdir && !SearchDirectory(find_path, is_in_subdir, filterfile))
                {
                    break;
                }
            }
            else
            {
                if (!filterfile(find_path, fd))
                {
                    break;
                }
            }
        }

        FindClose(find_handle);
        return true;
    }

    // »ñµÃµ±Ç°Ä£¿éÂ·¾¶Ãû
    void GetModuleName(char* module_name, int size)
    {
        if (NULL == module_name || 0 == size)
        {
            return;
        }

        HMODULE hModule = NULL;
        void* callerAddress = _ReturnAddress();
        if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (char*)callerAddress, &hModule))
        {
            GetModuleFileNameA(hModule, module_name, size);
        }
    }

    // µÃµ½Ä£¿éÄ¿Â¼
    bool GetLogDirectory(std::string& dir)
    {
        char module_dir[LOG_PATH_LEN] = { 0 };
        GetModuleName(module_dir, LOG_PATH_LEN);
        char* pFind = strrchr(module_dir, '\\');
        if (NULL != pFind)
        {
            *pFind = '\0';
            dir = module_dir;
            dir.append("\\log");
            return true;
        }
		
        return false;
    }

    // µÃµ½Ä£¿éÃû³Æ
    bool GetLogName(std::string& name)
    {
        char dir[LOG_PATH_LEN] = { 0 };
        GetModuleName(dir, LOG_PATH_LEN);
        char* pFind = strrchr(dir, '.');
        if (NULL != pFind)
        {
            *pFind = '\0';
            pFind = strrchr(dir, '\\');
            if (NULL != pFind)
            {
                name = (pFind + 1);
                return true;
            }
        }
        return false;
    }

    //»ñÈ¡µ±Ç°Ê±¼ä
    struct tm GetLocalTime()
    {
        time_t tNow = time(NULL);
        struct tm* ptm = localtime(&tNow);
        struct tm now = *ptm;
        ++now.tm_mon;
        now.tm_year += 1900;
        return now;
    }

    //////////////////////////////////////////////////////////////////////////

    struct LogFile::impl
    {
        FILE* log_fp_ptr_;
        CRITICAL_SECTION lock_;
        int ouput_level_;               // Êä³öÐÅÏ¢¼¶±ð
        int ouput_type_;                // ÈÕÖ¾Êä³öÀàÐÍ
        int reserve_day_;               // ÈÕÖ¾±£´æÌìÊý
        int log_size_;                  // ÈÕÖ¾´óÐ¡
        int log_index_;                 // ÈÕÖ¾Ë÷Òý
        struct tm log_time_;		    // µ±Ç°ÈÕÖ¾Ê±¼ä

        std::string directory_;         // ÈÕÖ¾Ä¿Â¼
        std::string log_name_;          // ÈÕÖ¾Ãû³Æ
        std::vector<char> buffer_a_;    // ÈÕÖ¾ÐÅÏ¢
        std::vector<wchar_t> buffer_w_; // ÈÕÖ¾ÐÅÏ¢

        impl()
            : log_fp_ptr_(NULL)
            , ouput_type_(LOG_DEF_TYPE)
            , ouput_level_(LOG_DEF_LEVEL)
            , reserve_day_(LOG_DEF_LIVE_DAY)
            , log_size_(LOG_DEF_FILE_SIZE)
            , log_index_(0)
            , buffer_a_(LOG_BUFFER_LEN, 0)
            , buffer_w_(LOG_BUFFER_LEN, 0)
        {
            ::InitializeCriticalSection(&lock_);
        }
        ~impl()
        {
            Uninitialize();
            ::DeleteCriticalSection(&lock_);
        }

        void lock()
        {
            EnterCriticalSection(&lock_);
        }

        void unlock()
        {
            LeaveCriticalSection(&lock_);
        }

        bool Initialize(int type, int level, int reserve_day, int size, const char* directory, const char* name)
        {
            if (0 != (type & LOG_DEF_TYPE)) ouput_type_ = type;
            if (0 != (level & LOG_DEF_LEVEL)) ouput_level_ = level;
            if (0 != reserve_day) reserve_day_ = reserve_day;
            if (0 != size) log_size_ = size;

            if (NULL != directory && 0 != strlen(directory))
            {
                directory_ = directory;
            }
            else
            {
                if (!GetLogDirectory(directory_))
                {
                    return false;
                }
            }

            if (NULL != name && 0 != strlen(name))
            {
                log_name_ = name;
            }
            else
            {
                if (!GetLogName(log_name_))
                {
                    return false;
                }
            }

            // ´´½¨Ä¿Â¼
            if (!CreateFullDirectory(directory_.c_str()))
            {
                return false;
            }

            setlocale(LC_ALL, "");

            // »ñµÃµ±Ç°Ê±¼ä
            log_time_ = GetLocalTime();

            // ÇåÀíÀÏµÄÈÕÖ¾
            ClearOldLog(log_time_);

            // µÃµ½×î´óË÷Òý£¬Èç¹ûÄ¾ÓÐ£¬ÔòÄ¬ÈÏÎª0
            log_index_ = GetMaxIndex(log_time_);

            // ´ò¿ªÈÕÖ¾ÎÄ¼þ
            return CreateLogFile(log_time_);
        }

        void Uninitialize()
        {
            if (NULL != log_fp_ptr_)
            {
                fclose(log_fp_ptr_);
                log_fp_ptr_ = NULL;
            }
        }

        // Êä³öANSI
        void OutputLogA(const char* trace, int level, const char* format, va_list argptr)
        {
            if (0 == (ouput_level_ & level))
            {
                return;
            }

            lock();

            struct tm *now = NULL;            

			timeb tb = {0};  
			ftime(&tb);
			now = localtime(&tb.time);
			int millisec = tb.millitm;
            int thread_id = GetCurrentThreadId();
            int pos = 0;
            pos += sprintf(&buffer_a_[pos],  HEADER_TIMESTAMP_A, now->tm_hour, now->tm_min, now->tm_sec, millisec);
            pos += sprintf(&buffer_a_[pos],  HEADER_THREADID_A, thread_id);
            pos += sprintf(&buffer_a_[pos],  HEADER_STRING_A, GetLevelStringA(level));
            if (NULL != trace)
            {
                pos += sprintf(&buffer_a_[pos],  HEADER_STRING_A, trace);
            }
            pos += sprintf(&buffer_a_[pos], "%s", " ");
            size_t buffer_size = _vscprintf(format, argptr) + pos + LOG_TAIL_LEN;
            if (buffer_size > buffer_a_.size())
            {
                size_t mult = GetMultiple(buffer_size, LOG_BUFFER_LEN);
                buffer_a_.resize(LOG_BUFFER_LEN*mult);
            }

            pos += _vsnprintf_s(&buffer_a_[pos], buffer_a_.size() - pos, _TRUNCATE, format, argptr);			
            sprintf(&buffer_a_[pos], "%s", TAIL_STRING_A);

            OuputByType();

            buffer_a_.assign(buffer_a_.size(), 0);

            unlock();
        }

        // Êä³öUNICODE
        void OutputLogW(const wchar_t* trace, int level, const wchar_t* format, va_list argptr)
        {
            if (0 == (ouput_level_ & level))
            {
                return;
            }

            lock();

            struct tm *now = NULL;          
			timeb tb = {0};  
			ftime(&tb);
			now = localtime(&tb.time);
			int millisec = tb.millitm;
            int thread_id = GetCurrentThreadId();
            int pos = 0;
            pos += swprintf(&buffer_w_[pos], buffer_w_.size() - pos, HEADER_TIMESTAMP_W, now->tm_hour, now->tm_min, now->tm_sec, millisec);
            pos += swprintf(&buffer_w_[pos], buffer_w_.size() - pos, HEADER_THREADID_W, thread_id);
            pos += swprintf(&buffer_w_[pos], buffer_w_.size() - pos, HEADER_STRING_W, GetLevelStringW(level));
            if (NULL != trace)
            {
                pos += swprintf(&buffer_w_[pos], buffer_w_.size() - pos, HEADER_STRING_W, trace);
            }
            pos += swprintf(&buffer_w_[pos], buffer_w_.size() - pos, L"%s", L" ");
            size_t buffer_size = _vscwprintf(format, argptr) + pos + LOG_TAIL_LEN;
            if (buffer_size > buffer_w_.size())
            {
                size_t mult = GetMultiple(buffer_size, LOG_BUFFER_LEN);
                buffer_w_.resize(LOG_BUFFER_LEN*mult);
                buffer_a_.resize(LOG_BUFFER_LEN*mult);
            }

            pos += _vsnwprintf_s(&buffer_w_[pos], buffer_w_.size() - pos, _TRUNCATE, format, argptr);
            swprintf(&buffer_w_[pos], buffer_w_.size() - pos, TAIL_STRING_W);

            WCharToMByte(&buffer_w_[0], &buffer_a_[0], buffer_a_.size());
            OuputByType();

            buffer_a_.assign(buffer_a_.size(), 0);
            buffer_w_.assign(buffer_w_.size(), 0);

            unlock();
        }

        // ¸ù¾ÝÊä³öÀàÐÍÊä³ö
        void OuputByType()
        {
            if (ouput_type_ & LogScreen)
            {
                printf("%s", buffer_a_.data());
            }
            if (ouput_type_ & LogVSDebugWindow)
            {
                OutputDebugStringA(buffer_a_.data());
            }
            if (ouput_type_ & LogDiskFile)
            {
                OutputDiskFile();
            }
        }

        // °ÑÈÕÖ¾ÐÅÏ¢Êä³öµ½ÎÄ¼þ
        void OutputDiskFile()
        {
            struct tm st = GetLocalTime();
            if (st.tm_mday != log_time_.tm_mday)
            {
                if (NULL != log_fp_ptr_)
                {
                    fclose(log_fp_ptr_);
                    log_fp_ptr_ = NULL;
                }

                ClearOldLog(st);

                // ÖØÖÃÊ±¼äºÍË÷Òý
                log_time_ = st;
                log_index_ = 0;
            }

            if (NULL == log_fp_ptr_)
            {
                CreateLogFile(st);
            }

            if (NULL != log_fp_ptr_)
            {
                int ret = fwrite(&buffer_a_[0], strlen(&buffer_a_[0]), 1, log_fp_ptr_);
				fflush(log_fp_ptr_);
                // Èç¹ûÎÄ¼þ´óÐ¡µ½¾¯½äÖµ£¬ÔòË÷Òý¼Ó1£¬¹Ø±Õµ±Ç°ÈÕÖ¾ÎÄ¼þ£¬´´½¨ÐÂµÄÈÕÖ¾ÎÄ¼þ
                if (IsFileOversize(log_fp_ptr_, log_size_))
                {
                    ++log_index_;
                    CreateLogFile(st);
                }
            }
        }

        // ´´½¨ÈÕÖ¾ÎÄ¼þ
        bool CreateLogFile(const struct tm& st)
        {
            char log_path[LOG_PATH_LEN] = { 0 };
            sprintf(log_path, FILE_NAME, directory_.c_str(), log_name_.c_str(), st.tm_year, st.tm_mon, st.tm_mday, log_index_);

            if (NULL != log_fp_ptr_)
            {
                fclose(log_fp_ptr_);
                log_fp_ptr_ = NULL;
            }

            log_fp_ptr_ = fopen(log_path, "a+");
            if (NULL != log_fp_ptr_)
            {
                return true;
            }
            return false;
        }

        // ÇåÀíÀÏµÄÈÕÖ¾ÎÄ¼þ
        void ClearOldLog(struct tm& st)
        {
            SearchDirectory(directory_.c_str(), false, [&](const char* file_path, const WIN32_FIND_DATAA& fd)->bool
            {
                if (0 == _strnicmp(fd.cFileName + strlen(fd.cFileName) - strlen(FILE_POSTFIX), FILE_POSTFIX, strlen(FILE_POSTFIX)))
                {
                    int diff_day = GetDiffDays(fd.ftCreationTime, st);
                    if (diff_day < 0 || diff_day > reserve_day_)
                    {
                        DeleteFileA(file_path);
                    }
                }
                return true;
            });
        }

        // µÃµ½µ±Ç°×î´óË÷Òý
        int GetMaxIndex(const struct tm& st)
        {
            int log_index = 0;
            unsigned long long file_size = 0;
            SearchDirectory(directory_.c_str(), false, [&](const char* file_path, const WIN32_FIND_DATAA& fd)->bool
            {
                std::string filename(fd.cFileName);
                if (filename.size() <= 15)
                {
                    return true;
                }

                size_t postfix_pos = filename.find(FILE_POSTFIX, 0);
                if (postfix_pos == std::string::npos)
                {
                    return true;
                }

                size_t name_pos = filename.find(log_name_, 0);
                if (name_pos == std::string::npos)
                {
                    return true;
                }

                SYSTEMTIME st_file;
                FILETIME ft_file = fd.ftCreationTime;
                FileTimeToSystemTime(&ft_file, &st_file);

                if (st.tm_year == st_file.wYear && st.tm_mon == st_file.wMonth && st.tm_mday == st_file.wDay)
                {
                    size_t separator = filename.find_last_of('_', postfix_pos) + 1;
                    if (separator == std::string::npos || 1 != (postfix_pos - separator))
                    {
                        return true;
                    }

                    std::string sub_str = filename.substr(separator, 1);
                    int index = atoi(sub_str.c_str());
                    if (log_index <= index)
                    {
                        log_index = index;
                        file_size = fd.nFileSizeLow + ((unsigned long long)fd.nFileSizeHigh << 32);
                    }
                }

                return true;
            });

            if (file_size >= (log_size_*MB_UNIT))
            {
                ++log_index;
            }

            return log_index;
        }

        // ÊÇ·ñÎÄ¼þ³¬¹ý¾¯½äÖµ
        bool IsFileOversize(FILE* fp, unsigned int size)
        {
            long file_size;
            if (!GetFileLength(fp, file_size))
            {
                return false;
            }

            if (file_size >= (long)(size*MB_UNIT))
            {
                return true;
            }
            return false;
        }

        const char* GetLevelStringA(int level)
        {
            static const char* level_string[] = { "Startup", "Shutdown", "Info", "Trace", "Debug", "Warning", "Error", "Fatal", "Unkown" };
            switch (level)
            {
                case LogStartup:
                    return level_string[0];
                case LogShutdown:
                    return level_string[1];
                case LogInfo:
                    return level_string[2];
                case LogTrace:
                    return level_string[3];
                case LogDebug:
                    return level_string[4];
                case LogWarning:
                    return level_string[5];
                case LogError:
                    return level_string[6];
                case LogFatal:
                    return level_string[7];
            }
            return level_string[8];
        }

        const wchar_t* GetLevelStringW(int level)
        {
            static const wchar_t* level_string[] = { L"Startup", L"Shutdown", L"Info", L"Trace", L"Debug", L"Warning", L"Error", L"Fatal", L"Unkown" };
            switch (level)
            {
                case LogStartup:
                    return level_string[0];
                case LogShutdown:
                    return level_string[1];
                case LogInfo:
                    return level_string[2];
                case LogTrace:
                    return level_string[3];
                case LogDebug:
                    return level_string[4];
                case LogWarning:
                    return level_string[5];
                case LogError:
                    return level_string[6];
                case LogFatal:
                    return level_string[7];
            }
            return level_string[8];
        }

        bool EmptyLogFile()
        {
            bool ret = false;

            lock();

            if (NULL != log_fp_ptr_)
            {
                fclose(log_fp_ptr_);
                log_fp_ptr_ = NULL;
            }

            char log_path[LOG_PATH_LEN] = { 0 };
            sprintf(log_path, FILE_NAME, directory_.c_str(), log_name_.c_str(), log_time_.tm_year, log_time_.tm_mon, log_time_.tm_mday, log_index_);

            if (DelFile(log_path))
            {
                ret = CreateLogFile(log_time_);
            }

            unlock();

            return ret;
        }
    };
	
	
    //////////////////////////////////////////////////////////////////////////

    LogFile::LogFile(void):impl_(NULL)
    {
    }

    LogFile::~LogFile(void)
    {
    }

    bool LogFile::Initialize(int type, int level, int reserve_day, int size, const char* directory, const char* name)
    {
        if (NULL != impl_)
        {
            return false;
        }

        impl_ = new impl;
        if (NULL == impl_)
        {
            return false;
        }

        if (!impl_->Initialize(type, level, reserve_day, size, directory, name))
        {
            delete impl_;
			impl_ = NULL;
            return false;
        }

        return true;
    }

    void LogFile::Uninitialize()
    {
        if (NULL != impl_)
        {
            impl_->Uninitialize();
            delete impl_;
			impl_= NULL;
        }
    }

    void LogFile::OutputLog(int level, const char* format, ...)
    {
        va_list Args;
        va_start(Args, format);
        if (NULL != impl_)
        {
            impl_->OutputLogA(NULL, level, format, Args);
        }
        va_end(Args);
    }

    void LogFile::OutputLog(int level, const wchar_t* format, ...)
    {
        va_list Args;
        va_start(Args, format);
        if (NULL != impl_)
        {
            impl_->OutputLogW(NULL, level, format, Args);
        }
        va_end(Args);
    }

    void LogFile::OutputLog(const char* trace, int level, const char* format, ...)
    {
        va_list Args;
        va_start(Args, format);
        if (NULL != impl_)
        {
            impl_->OutputLogA(trace, level, format, Args);
        }
        va_end(Args);
    }

    void LogFile::OutputLog(const wchar_t* trace, int level, const wchar_t* format, ...)
    {
        va_list Args;
        va_start(Args, format);
        if (NULL != impl_)
        {
            impl_->OutputLogW(trace, level, format, Args);
        }
        va_end(Args);
    }


    void LogFile::OutputArgLog(int level, const char* format, va_list argptr)
    {
        if (NULL != impl_)
        {
            impl_->OutputLogA(NULL, level, format, argptr);
        }
    }
    void LogFile::OutputArgLog(int level, const wchar_t* format, va_list argptr)
    {
        if (NULL != impl_)
        {
            impl_->OutputLogW(NULL, level, format, argptr);
        }
    }
    void LogFile::OutputArgLog(const char* trace, int level, const char* format, va_list argptr)
    {
        if (NULL != impl_)
        {
            impl_->OutputLogA(trace, level, format, argptr);
        }
    }
    void LogFile::OutputArgLog(const wchar_t* trace, int level, const wchar_t* format, va_list argptr)
    {
        if (NULL != impl_)
        {
            impl_->OutputLogW(trace, level, format, argptr);
        }
    }

    bool LogFile::EmptyLogFile()
    {
        if (NULL != impl_)
        {
            return impl_->EmptyLogFile();
        }

        return false;
    }

    //////////////////////////////////////////////////////////////////////////

    ModuleLog::ModuleLog(const char* module_name)
    {
        strcpy(module_name_, module_name);
    }

    ModuleLog::~ModuleLog(void)
    {
    }

    void ModuleLog::TraceLog(int level, const char* format, ...) const
    {
        va_list Args;
        va_start(Args, format);
        LogFile::Instance().OutputArgLog(module_name_, level, format, Args);
        va_end(Args);
    }

    void ModuleLog::TraceLog(int level, const wchar_t* format, ...) const
    {
        wchar_t module_name[260] = { 0 };
        MByteToWChar(module_name_, module_name, 260);

        va_list Args;
        va_start(Args, format);
        LogFile::Instance().OutputArgLog(module_name, level, format, Args);
        va_end(Args);
    }

}