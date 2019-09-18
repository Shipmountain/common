/*******************************************************************************
 * Ëµ    Ã÷£º±¾Ä£¿éÎªµ¥Àý£¬
 *
 *           ÔÚ´òÓ¡Ç°ÐèÒªµ÷ÓÃInitLog³õÊ¼»¯£¬Æä²ÎÊýÎª
 *              type£º       ÈÕÖ¾Êä³öÀàÐÍ£¬Ä¬ÈÏÊä³öËùÓÐÀàÐÍ
 *              level£º      ÈÕÖ¾Êä³ö¼¶±ð£¬Ä¬ÈÏÊä³öËùÓÐ¼¶±ð
 *              reserve_day£ºÈÕÖ¾´æ»îÌìÊý£¬Ä¬ÈÏ10Ìì
 *              size£º       ÈÕÖ¾×î´ó´óÐ¡£¬µ¥Î»ÎªM£¬Ä¬ÈÏ1024M
 *              directory£º  ÈÕÖ¾Êä³öÄ¿Â¼£¬Ä¬ÈÏÎªµ±Ç°Ä£¿é*\\log
 *              name£º       ÈÕÖ¾Êä³öÃû³Æ£¬Ä¬ÈÏÎªµ±Ç°Ä£¿éÃû³Æ
 *              ÈôÅäÖÃ²ÎÊýÎª0£¬Ôò»áÅäÖÃ³ÉÄ¬ÈÏÖµ
 *              Àý×Ó£º
 *                   InitLog(0, 0, 0, 0, NULL, NULL);
 *
 *           GenericLogÎªÈÕÖ¾Êä³ö
 *              Àý×Ó£º
 *                   ÆÕÍ¨ÐÅÏ¢´òÓ¡£ºGenericLog(LogInfo, "%s", "ok"); ´òÓ¡½á¹ûÎª£ºok
 *                   ¸ú×ÙÐÅÏ¢´òÓ¡£ºGenericLog("trace", LogInfo, "%s", "ok");´òÓ¡½á¹ûÎª£º[trace] ok
 *           ÔÚ³ÌÐò½áÊøÊ±µ÷ÓÃUninitLogÐ¶ÔØ
 *
 * ±¸    ×¢£º´ò¿ªÎÄ¼þÎª¶ÀÐ´Ä£Ê½£¬Ö»ÓÐÓµÓÐÄ£¿é²ÅÄÜÊäÈëÈÕÖ¾£¬
 *           Òò´ËÓÐ¼ÓÔØµÄDLLÐèÒªÔÚ±¾Ä£¿éÉÏ´òÓ¡ÈÕÖ¾£¬ÐèÒªÌá¹©´òÓ¡ÈÕÖ¾»Øµ÷£¬
 *           ÈôÐèÒªµ¥¶À´òÓ¡ÈÕÖ¾£¬Ôòµ¥¶ÀÅäÖÃname
 ******************************************************************************/

#ifndef _LOG_H_
#define _LOG_H_

#include <stdio.h>
#include <windows.h>
#include <memory>

//////////////////////////////////////////////////////////////////////////

enum LogOutputLevel         // ÈÕÖ¾Êä³ö¼¶±ð
{
    LogStartup = 1,		// Æô¶¯ÐÅÏ¢£ºÃèÊöÄ£¿éµÄÆô¶¯
    LogShutdown = 2,		// ¹Ø±ÕÐÅÏ¢£ºÃèÊöÄ£¿éµÄ¹Ø±Õ
    LogInfo = 4,		// ÆÕÍ¨ÐÅÏ¢£º³£Ì¬´òÓ¡£¬°üÀ¨Æô¶¯/Í£Ö¹ÐÅÏ¢...
    LogWarning = 8,		// ¾¯¸æÐÅÏ¢£º³ÌÐòÔËÐÐ¾¯¸æ×´Ì¬
    LogTrace = 16,		// ¸ú×ÙÐÅÏ¢£ºÒµÎñ¹ý³Ì¸ú×Ù
    LogError = 32,		// ´íÎóÐÅÏ¢£º²»Ó°Ïì³ÌÐòÔËÐÐµÄ´íÎó
    LogDebug = 64,		// µ÷ÊÔÐÅÏ¢£ºÖ»ÓÃÓÚDebug°æ±¾
    LogFatal = 128       // ÖÂÃüÐÅÏ¢£ºµ¼ÖÂ³ÌÐòÎÞ·¨¼ÌÐøÔËÐÐµÄ´íÎó
};

enum LogOutputType          // ÈÕÖ¾Êä³öÀàÐÍ
{
    LogScreen = 1,			// ÆÁÄ»Êä³ö
    LogVSDebugWindow = 2,	// VCµÄDebugWindowÊä³ö
    LogDiskFile = 4,		// ÎÄ¼þÊä³ö
};

// ÈÕÖ¾Ä£¿é¶ÔÍâ³£ÓÃ½Ó¿Ú
#define InitLog         Log::LogFile::Instance().Initialize
#define UninitLog       Log::LogFile::Instance().Uninitialize
#define GenericLog      Log::LogFile::Instance().OutputLog
#define EmptyLog        Log::LogFile::Instance().EmptyLogFile
#define GenericArgLog   Log::LogFile::Instance().OutputArgLog


//////////////////////////////////////////////////////////////////////////
// ÈÕÖ¾ÊµÏÖ

namespace Log
{
    class LogFile
    {
	public:
        struct impl;
	private:
        impl* impl_;

    public:
        LogFile(void);
        ~LogFile(void);

        static LogFile& Instance()
        {
            static LogFile obj;
            return obj;
        }

        /*
        * ¹¦    ÄÜ£ºÈÕÖ¾³õÊ¼»¯
        * ²Î    Êý£ºtype£º       ÈÕÖ¾Êä³öÀàÐÍ£¬Ä¬ÈÏÊä³öËùÓÐÀàÐÍ
        *           level£º      ÈÕÖ¾Êä³ö¼¶±ð£¬Ä¬ÈÏÊä³öËùÓÐ¼¶±ð
        *           reserve_day£ºÈÕÖ¾´æ»îÌìÊý£¬Ä¬ÈÏ10Ìì
        *           size£º       ÈÕÖ¾×î´ó´óÐ¡£¬µ¥Î»ÎªM£¬Ä¬ÈÏ1024M
        *           directory£º  ÈÕÖ¾Êä³öÄ¿Â¼£¬Ä¬ÈÏÎªµ±Ç°Ä£¿é*\\log
        *           name£º       ÈÕÖ¾Êä³öÃû³Æ£¬Ä¬ÈÏÎªµ±Ç°Ä£¿éÃû³Æ
        * ·µ »Ø Öµ£ºÈç¹û³É¹¦Ôò·µ»Øtrue£¬·ñÔò·µ»Øfalse
        * ±¸    ×¢£ºÈç¹ûÅäÖÃ²ÎÊýÎª0£¬Ôò»áÅäÖÃ³ÉÄ¬ÈÏÖµ
        */
        bool Initialize(int type, int level, int reserve_day, int size, const char* directory, const char* name);

        /*
        * ¹¦    ÄÜ£ºÈÕÖ¾Ð¶ÔØ£¬ÊÍ·ÅÄÚ²¿×ÊÔ´
        * ²Î    Êý£º
        * ·µ »Ø Öµ£º
        */
        void Uninitialize();

        void OutputLog(int level, const char* format, ...);
        void OutputLog(int level, const wchar_t* format, ...);

        void OutputLog(const char* trace, int level, const char* format, ...);
        void OutputLog(const wchar_t* trace, int level, const wchar_t* format, ...);

        void OutputArgLog(int level, const char* format, va_list argptr);
        void OutputArgLog(int level, const wchar_t* format, va_list argptr);

        void OutputArgLog(const char* trace, int level, const char* format, va_list argptr);
        void OutputArgLog(const wchar_t* trace, int level, const wchar_t* format, va_list argptr);

        //½«ÈÕÖ¾ÎÄ¼þÄÚÈÝÇå¿Õ
        bool EmptyLogFile();

    private:
        LogFile(const LogFile&);
        LogFile& operator=(const LogFile&);
    };


    //////////////////////////////////////////////////////////////////////////


    // Ä£¿éÈÕÖ¾
    class ModuleLog
    {
    public:
        ModuleLog(const char* module_name);
        ~ModuleLog(void);

        void TraceLog(int level, const char* format, ...) const;
        void TraceLog(int level, const wchar_t* format, ...) const;

    private:
        ModuleLog(void){}
        ModuleLog(const ModuleLog&);
        ModuleLog& operator=(const ModuleLog&);

        char module_name_[260];
    };


}// namespace log

#endif // _LOG_H_
