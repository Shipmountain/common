#ifndef _CHS_TO_PINYIN_H_
#define _CHS_TO_PINYIN_H_
#include <string>

namespace comm
{
    enum EPinyinMode{
        EPinyinModeAllUpper = 0,        //全大写
        EPinyinModeAllLower,            //全小写
        EPinyinModeFirstUpper,          //首字母大写
    };

    unsigned get_pinyin(const char* chs_ptr, char* buf, const unsigned buf_len, EPinyinMode mode);

    std::string get_first_letter(const std::string& chs_str);
}

#endif //end _CHS_TO_PINYIN_H_