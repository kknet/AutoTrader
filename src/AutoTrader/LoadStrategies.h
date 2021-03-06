#include <vector>
#include <boost/filesystem.hpp>

//#include <stdio.h>
//#include <stdlib.h>
#ifdef WIN32
#include <Windows.h>
#define DLLPTR HMODULE 
#define OpenDLL LoadLibrary
#define GetFuncPtr GetProcAddress 
#define CloseDLL FreeLibrary 
#else
#include <dlfcn.h>
#define DLLPTR void*
#define OpenDLL dlopen
#define GetFuncPtr dlsym
#define CloseDLL dlclose
#endif
namespace fs = boost::filesystem;
typedef void(*loadFunc)(void);



struct StrategyPluginsLoader
{
	StrategyPluginsLoader(){
		std::string fullpath = fs::initial_path<fs::path>().string();
		fs::directory_iterator end_iter;
		for (fs::directory_iterator file_itr(fullpath); file_itr != end_iter; ++file_itr)
		{
			if (!fs::is_directory(*file_itr) && (fs::extension(*file_itr) == ".stg"))
			{
#ifdef WIN32
				DLLPTR handle = OpenDLL(file_itr->path().wstring().c_str());
#else
				DLLPTR handle = OpenDLL(file_itr->path().string().c_str(), RTLD_GLOBAL | RTLD_NOW);
#endif
				m_handles.push_back(handle);
			}
		}

		_CallEntryFunction();
	}

	~StrategyPluginsLoader(){
		_FreeLibraries();
	}
private:

	void _CallEntryFunction(){
		for (DLLPTR handle : m_handles){
			loadFunc funPtr = (loadFunc)GetFuncPtr(handle, "LoadPlugin");
			funPtr();
		}
	}

	void _FreeLibraries(){
		for (DLLPTR handle : m_handles){
			loadFunc funPtr = (loadFunc)GetFuncPtr(handle, "FreePlugin");
			funPtr();
			CloseDLL(handle);
		}
	}

private:
	std::vector<DLLPTR> m_handles;
};
