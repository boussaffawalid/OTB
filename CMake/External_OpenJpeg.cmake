message(STATUS "Setup OpenJpeg...")

set(proj OPENJPEG)

set(DEFAULT_USE_SYSTEM_OPENJPEG  OFF)

option(USE_SYSTEM_OPENJPEG "  Use a system build of OpenJpeg." ${DEFAULT_USE_SYSTEM_OPENJPEG})
mark_as_advanced(USE_SYSTEM_OPENJPEG)

if(USE_SYSTEM_OPENJPEG)
  message(STATUS "  Using OpenJpeg system version")
else()
  set(${proj}_DEPENDENCIES)
  set(OPENJPEG_SB_BUILD_DIR ${CMAKE_BINARY_DIR}/${proj}/build)
  set(OPENJPEG_SB_SRC ${CMAKE_BINARY_DIR}/${proj}/src/${proj})
  
  # handle dependencies : TIFF, ZLIB, ...
  # although they seem un-used by the openjp2 codec
  if(USE_SYSTEM_TIFF)
    # TODO : handle specific prefix
  else()
    list(APPEND ${proj}_DEPENDENCIES TIFF)
  endif()
  
  if(USE_SYSTEM_ZLIB)
    # TODO : handle specific prefix
  else()
    list(APPEND ${proj}_DEPENDENCIES ZLIB)
  endif()

  if(MSVC)
  #TODO: add LCMS dependency
  endif()
  
  ExternalProject_Add(${proj}
        PREFIX ${proj}
        URL "http://sourceforge.net/projects/openjpeg.mirror/files/2.0.0/openjpeg-2.0.0.tar.gz/download"
        URL_MD5 d9be274bddc0f47f268e484bdcaaa6c5
        BINARY_DIR ${OPENJPEG_SB_BUILD_DIR}
        INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
        CMAKE_CACHE_ARGS
        -DCMAKE_INSTALL_PREFIX:STRING=${CMAKE_INSTALL_PREFIX}
        -DCMAKE_BUILD_TYPE:STRING=Release
        -DBUILD_CODEC:BOOL=ON
        -DBUILD_DOC:BOOL=OFF
        -DBUILD_JPIP:BOOL=OFF
        -DBUILD_JPWL:BOOL=OFF
        -DBUILD_MJ2:BOOL=OFF
        -DBUILD_PKGCONFIG_FILES:BOOL=ON
        -DBUILD_SHARED_LIBS:BOOL=ON
        -DBUILD_TESTING:BOOL=OFF
        -DBUILD_THIRDPARTY:BOOL=OFF
        -DCMAKE_PREFIX_PATH:STRING=${CMAKE_INSTALL_PREFIX}
        DEPENDS ${${proj}_DEPENDENCIES}
        CMAKE_COMMAND ${SB_CMAKE_COMMAND}
    )

  message(STATUS "  Using OpenJPEG SuperBuild version")

 
endif()
