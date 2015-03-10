message(STATUS "Setup cURL ...")

set(proj CURL)

if(NOT __EXTERNAL_${proj}__)
set(__EXTERNAL_${proj}__ 1)

set(DEFAULT_USE_SYSTEM_CURL  ON)

option(USE_SYSTEM_CURL "  Use a system build of cURL." ${DEFAULT_USE_SYSTEM_CURL})
mark_as_advanced(USE_SYSTEM_CURL)

if(USE_SYSTEM_CURL)
  message(STATUS "  Using cURL system version")
else()
  SETUP_SUPERBUILD(PROJECT ${proj})
    
  if(USE_SYSTEM_ZLIB)
    set(CURL_SB_ZLIB_CONFIG)
  else()
    set(CURL_SB_ZLIB_CONFIG 
      -DZLIB_ROOT:STRING=${SB_INSTALL_PREFIX}
      )
    list(APPEND ${proj}_DEPENDENCIES ZLIB)
  endif()
  
  #TODO: add openssl and other dependencies
  if(MSVC)
    ExternalProject_Add(${proj}
        PREFIX ${proj}
        URL "http://curl.haxx.se/download/curl-7.40.0.tar.gz"
        URL_MD5 58943642ea0ed050ab0431ea1caf3a6f
        SOURCE_DIR ${CURL_SB_SRC}
        BINARY_DIR ${CURL_SB_BUILD_DIR}/winbuild
        INSTALL_DIR ${SB_INSTALL_PREFIX}
        DEPENDS ${${proj}_DEPENDENCIES}        
        PATCH_COMMAND ${CMAKE_COMMAND} -E copy_directory ${CURL_SB_SRC} ${CURL_SB_BUILD_DIR}
        CONFIGURE_COMMAND ""
        BUILD_COMMAND nmake /f ${CURL_SB_BUILD_DIR}/winbuild/Makefile.vc mode=dll WITH_ZLIB=dll WITH_DEVEL=${SB_INSTALL_PREFIX}
        INSTALL_COMMAND ${CMAKE_COMMAND} -E chdir ${CURL_SB_BUILD_DIR}/builds/ ${CMAKE_COMMAND} -E copy_directory libcurl-vc-x86-release-dll-zlib-dll-ipv6-sspi-winssl ${SB_INSTALL_PREFIX} 
    )
    
  else(UNIX)
    ExternalProject_Add(${proj}
        PREFIX ${proj}
        URL "http://curl.haxx.se/download/curl-7.40.0.tar.gz"
        URL_MD5 58943642ea0ed050ab0431ea1caf3a6f
        BINARY_DIR ${CURL_SB_BUILD_DIR}
        INSTALL_DIR ${SB_INSTALL_PREFIX}
        CMAKE_CACHE_ARGS
        -DCMAKE_INSTALL_PREFIX:STRING=${SB_INSTALL_PREFIX}
        -DCMAKE_BUILD_TYPE:STRING=Release
        -DBUILD_SHARED_LIBS:BOOL=ON
        -DBUILD_CURL_EXE:BOOL=ON
        -DBUILD_CURL_TESTS:BOOL=OFF
        ${CURL_SB_ZLIB_CONFIG}
        DEPENDS ${${proj}_DEPENDENCIES}
    )
  endif()
  message(STATUS "  Using cURL SuperBuild version")

endif()
endif()
