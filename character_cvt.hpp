#ifndef __UTF8_WCHAR_MCHAR_INC__
#define __UTF8_WCHAR_MCHAR_INC__

#include <windows.h>
#include <vector>
#include <string>

namespace character
{
    //utf8 转 Unicode
    static inline std::wstring utf8_to_unicode(const char* utf8)
    {
        int size = ::MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
        if (size == ERROR_NO_UNICODE_TRANSLATION)
        {
            throw std::exception("Invalid UTF-8 sequence.");
        }
        if (size == 0)
        {
            throw std::exception("Error in conversion.");
        }

        std::vector<wchar_t> wstr(size);
        int convresult = ::MultiByteToWideChar(CP_UTF8, 0, utf8, -1, &wstr[0], size);
        if (convresult != size)
        {
            throw std::exception("La falla!");
        }

        return std::move(std::wstring(&wstr[0]));
    }

    //unicode 转为 multi-byte
    static inline std::string unicode_to_mbyte(const wchar_t* wstr)
    {
        int size = ::WideCharToMultiByte(CP_OEMCP, 0, wstr, -1, NULL, 0, NULL, NULL);
        if (size == ERROR_NO_UNICODE_TRANSLATION)
        {
            throw std::exception("Invalid UTF-8 sequence.");
        }
        if (size == 0)
        {
            throw std::exception("Error in conversion.");
        }
        std::vector<char> str(size);
        int convresult = ::WideCharToMultiByte(CP_OEMCP, 0, wstr, -1, &str[0], size, NULL, NULL);
        if (convresult != size)
        {
            throw std::exception("La falla!");
        }

        return std::move(std::string(&str[0]));
    }

    //utf-8 转 multi-byte
    static inline std::string utf8_to_mbyte(const char* utf8)
    {
        //先把 utf8 转为 unicode 
        std::wstring wstr = utf8_to_unicode(utf8);

        //最后把 unicode 转为 multi-byte
        std::string str = unicode_to_mbyte(wstr.c_str());

        return std::move(str);
    }

    ///////////////////////////////////////////////////////////////////////

    //multi-byte 转 Unicode
    static inline std::wstring mbyte_to_unicode(const char* str)
    {
        int size = MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, 0);
        if (size == ERROR_NO_UNICODE_TRANSLATION)
        {
            throw std::exception("Invalid UTF-8 sequence.");
        }
        if (size == 0)
        {
            throw std::exception("Error in conversion.");
        }
        std::vector<wchar_t> wstr(size);
        int convresult = MultiByteToWideChar(CP_ACP, 0, str, -1, &wstr[0], size);
        if (convresult != size)
        {
            throw std::exception("La falla!");
        }

        return std::move(std::wstring(&wstr[0]));
    }

    //Unicode 转 Utf8
    static inline std::string unicode_to_utf8(const wchar_t* wstr)
    {
        int size = ::WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
        if (size == 0)
        {
            throw std::exception("Error in conversion.");
        }

        std::vector<char> utf8(size);
        int convresult = ::WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &utf8[0], size, NULL, NULL);
        if (convresult != size)
        {
            throw std::exception("La falla!");
        }

        return std::move(std::string(&utf8[0]));
    }

    //multi-byte 转 Utf8
    static inline std::string mbyte_to_utf8(const char* str)
    {
        //先把 multi-byte 转为 unicode 
        std::wstring wstr = mbyte_to_unicode(str);

        //最后把 unicode 转为 utf8

        std::string ret = unicode_to_utf8(wstr.c_str());

        return std::move(ret);
    }

}

#endif