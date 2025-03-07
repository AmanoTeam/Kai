#if defined(_WIN32)

#if defined(_WIN32)
	#include <windows.h>
	#include <libloaderapi.h>
#endif

#include "os/emulated.h"

#if defined(_WIN32)
	#if defined(_UNICODE)
		static const wchar_t WNTDLL[] = L"ntdll.dll";
	#else
		static const char NTDLL[] = "ntdll.dll";
	#endif
#endif

#if defined(_WIN32)
	int is_wine(void) {
		/*
		Returns whether the caller's process is running under Wine.
		
		Returns (1) on true, (0) on false, (-1) on error.
		*/
		
		#if defined(_UNICODE)
			HMODULE module = GetModuleHandleW(WNTDLL);
		#else
			HMODULE module = GetModuleHandleA(NTDLL);
		#endif
		
		if (module == NULL) {
			return -1;
		}
		
		if (GetProcAddress(module, "wine_get_version") == NULL) {
			return 0;
		}
		
		return 1;
		
	}
#endif

#endif
