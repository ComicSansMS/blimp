if(MSVC)
    find_path(VLD_INCLUDE_DIR NAMES vld.h
        HINTS ENV VISUAL_LEAK_DETECTOR_ROOT
        PATH_SUFFIXES include
    )

    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(WIN_SUFFIX Win64)
    else()
        set(WIN_SUFFIX Win32)
    endif()

    find_library(VLD_LIBRARY NAMES vld
        HINTS ENV VISUAL_LEAK_DETECTOR_ROOT
        PATH_SUFFIXES lib/${WIN_SUFFIX}
    )

    find_file(VLD_LIBRARY_DLL vld_x86.dll vld_x64.dll
        HINTS ENV VISUAL_LEAK_DETECTOR_ROOT
        PATH_SUFFIXES bin/${WIN_SUFFIX}
    )

    find_file(VLD_DBGHLP_DLL dbghelp.dll
        HINTS ENV VISUAL_LEAK_DETECTOR_ROOT
        PATH_SUFFIXES bin/${WIN_SUFFIX}
    )

    find_file(VLD_MANIFEST_FILE Microsoft.DTfW.DHL.manifest
        HINTS ENV VISUAL_LEAK_DETECTOR_ROOT
        PATH_SUFFIXES bin/${WIN_SUFFIX}
    )

    include(FindPackageHandleStandardArgs)

    find_package_handle_standard_args(VLD
        DEFAULT_MSG
        VLD_INCLUDE_DIR
        VLD_LIBRARY
        VLD_LIBRARY_DLL
        VLD_DBGHLP_DLL
        VLD_MANIFEST_FILE
    )

    if(VLD_FOUND)
        set(VLD_RUNTIME_FILES ${VLD_LIBRARY_DLL} ${VLD_DBGHLP_DLL} ${VLD_MANIFEST_FILE} CACHE INTERNAL "")
    endif()
endif()
