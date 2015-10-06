file(GLOB_RECURSE MemoryAllocatorSources
	"*.h"
	"*.cpp"
)
add_definitions(-DMEMORYALLOCATOR_DLL_EXPORT)
add_library(MemoryAllocator SHARED ${MemoryAllocatorSources})

install(
	TARGETS MemoryAllocator DESTINATION lib
	LIBRARY DESTINATION lib
	RUNTIME DESTINATION bin/${OperatingSystemNameLowerCase}/${TargetArchitecture})
