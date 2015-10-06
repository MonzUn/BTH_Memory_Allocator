#ifdef _WIN32
	#ifdef MEMORYALLOCATOR_DLL_EXPORT
		#define MEMORYALLOCATOR_API __declspec(dllexport)
	#else
		#define MEMORYALLOCATOR_API __declspec(dllimport)
	#endif
#else
	#define MEMORYALLOCATOR_API
#endif
