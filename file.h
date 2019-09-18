#ifndef _FILE_H_
#define _FILE_H_

#include <windows.h>
#include <io.h>
#include <string>
#include <deque>
#include <ShlObj.h>

#pragma comment(lib,"shell32.lib")

namespace file
{
	typedef bool(*cb_func)(const std::string& file, const std::string file_name, bool is_dir, void* cb_param);

    static bool file_exist(const std::string& file)
    {
        return _access(file.c_str(), 0) == 0;
    }

    static void walk_dir(const std::string& dir, cb_func func, void* cb_param, unsigned depth = 0)
	{
        if (NULL == func || dir.empty() || !file_exist(dir)) return;

        std::string path = dir;
        char ch = path[path.size() - 1];
        if ('\\'== ch || '/' == ch)
        {
            path.resize(path.size() - 1);
        }

        struct DirInfo
        {
            DirInfo() :depth_(0){};
            DirInfo(const std::string& dir, unsigned depth) :dir_(dir), depth_(depth){};
            std::string dir_;
            unsigned    depth_;
        };

        std::deque<DirInfo> dir_deque;
        dir_deque.push_back(DirInfo(path, 1));
        WIN32_FIND_DATAA find_data = { 0 };
        while (!dir_deque.empty())
        {
            DirInfo dir_info = dir_deque.front();
            dir_deque.pop_front();

            memset(&find_data, 0, sizeof(WIN32_FIND_DATAA));
            path = dir_info.dir_ + "\\*";
            HANDLE find_handle = FindFirstFileA(path.c_str(), &find_data);
            if (NULL == find_handle) continue;

            do{
                if (0 == strcmp(find_data.cFileName, ".") || 
                    0 == strcmp(find_data.cFileName, ".."))  continue;

                std::string file_path = dir_info.dir_ + "\\" + find_data.cFileName;
                if ((find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
                    (0 == depth || dir_info.depth_ < depth))
                {
                    dir_deque.push_back(DirInfo(file_path, dir_info.depth_ + 1));
                }

                func(file_path, find_data.cFileName, find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY, cb_param);

            } while (FindNextFileA(find_handle, &find_data));
            
            FindClose(find_handle);
        }
	}

	static bool delete_file(const std::string& file)
	{
		return ::DeleteFileA(file.c_str())?true:false;
	}

	static bool create_folder(const std::string& folder)
	{
        if (file_exist(folder))
		{
			return true;
		}

        std::string dir = folder;
        for (std::string::iterator itor = dir.begin(); itor != dir.end(); ++itor)
        {
            if ('/' == *itor)
            {
                *itor = '\\';
            }
        }

		return ::SHCreateDirectoryExA(NULL, dir.c_str(), NULL) == 0;
	}

	static bool delete_dir(const std::string& dir)
	{
        if (dir.empty()) return false;

        if (!file_exist(dir)) return true;

        std::string path = dir;
        char ch = path[path.size() - 1];
        while ('\\' == ch || '/' == ch)
        {
            path.erase(path.size() - 1);
            ch = path[path.size() - 1];
        }
        path[path.size() + 1] = 0;

        SHFILEOPSTRUCTA file_op;
        ZeroMemory((void*)&file_op, sizeof(SHFILEOPSTRUCTA));
        file_op.fFlags = FOF_NOCONFIRMATION;
        file_op.pFrom = path.c_str();
        file_op.wFunc = FO_DELETE;

        return SHFileOperationA(&file_op) == 0;
	}

	static bool move_file(const std::string& src, const std::string& dst)
	{
		return ::MoveFileA(src.c_str(), dst.c_str())?true:false;
	}

	static bool copy_file(const std::string& src, const std::string& dst)
	{
		return ::CopyFileA(src.c_str(), dst.c_str(), FALSE)?true:false;
	}
};

#endif  // _FILE_H_