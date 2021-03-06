#
# Current informations:
# Version 1.0B released on May, 2005.
# Download site : http://6s.ltdri.org/
#

project(OTB6S C)

# Allow sources in subdirectories to see the include files.
include_directories(${OTB6S_SOURCE_DIR})

# Inform header files they are being included from a source inside the
# otb_6S library.
add_definitions(-DOTB_6S_SRC)
add_definitions(-DNON_POSIX_STDIO)

# On Microsoft Visual
if(WIN32)
  if(NOT BORLAND)
    if(NOT CYGWIN)
      if(NOT MINGW)
#        if(NOT ITK_ENABLE_VISUAL_STUDIO_DEPRECATED_C_WARNINGS)
          add_definitions(
            -DUSE_CLOCK
            -DMSDOS
            -DNO_ONEXIT
            )
#        endif()
      else()
          add_definitions(
            -DUSE_CLOCK
            -DMSDOS
            -DNO_TRUNCATE  #For generate endfile.c
            )
      endif()
    endif()
  endif()
endif()


# source files for otb6S
file(GLOB OTB6S_SRCS "6SV1.0B/*.c" )

# source files for libf2c
# Not use itkf2c because uncomplete for link 6S main exectuable
file(GLOB LIBF2C_SRCS "libf2c/*.c" )

list(REMOVE_ITEM LIBF2C_SRCS "${OTB6S_SOURCE_DIR}/libf2c/arithchk.c" )
list(REMOVE_ITEM LIBF2C_SRCS "${OTB6S_SOURCE_DIR}/libf2c/main.c" )
list(REMOVE_ITEM LIBF2C_SRCS "${OTB6S_SOURCE_DIR}/libf2c/getarg_.c" )
list(REMOVE_ITEM LIBF2C_SRCS "${OTB6S_SOURCE_DIR}/libf2c/iargc_.c" )

if(CMAKE_COMPILER_IS_GNUCXX)
  foreach(f ${LIBF2C_SRCS} )
        set_source_files_properties( ${f} PROPERTIES COMPILE_FLAGS -w )
  endforeach()
else()
  if(WIN32)
    if(CMAKE_CXX_COMPILER MATCHES "^cl$")
        foreach(f ${LIBF2C_SRCS} )
                set_source_files_properties( ${f} PROPERTIES COMPILE_FLAGS "/W0" )
        endforeach()
    endif()
  endif()
endif()


# Main code source
set(OTB6S_MAIN_SRCS
otb_main6S_function.c                # otb 6S main function
main.c                               # ORIGINAL 6S main
otb_main6S_program_call_function.c   # OTB 6S main (which call otb_main6S_function function)
aeroprof_com.c                       # others *.c fils generated by f2c
multorder_com.c
sixs_del__com.c
sixs_ier__com.c
num_quad__com.c
sixs_aer__com.c
sixs_disc__com.c
sixs_planesim__com.c
mie_in__com.c
sixs_atm__com.c
sixs_ffu__com.c
sixs_test__com.c
)

include_directories(${OTB6S_SOURCE_DIR}/libf2c)

# Adjust the compiler flags to avoid problems with f2c-generated code.
if(CMAKE_COMPILER_IS_GNUCC)
  # Hide warnings in f2c-generated code.
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -w")

  # CMake versions before 2.4 did not put the source-file-specific flags
  # after the directory-level flags.  Remove optimization flags from the
  # CMAKE_C_FLAGS* variables so they can be overridden.
  if("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" LESS 2.4)
    foreach(c "" "_DEBUG" "_RELEASE" "_MINSIZEREL" "_RELWITHDEBINFO")
      string(REGEX REPLACE
        "-O[^ ]*( |$)" "" CMAKE_C_FLAGS${c} "${CMAKE_C_FLAGS${c}}")
    endforeach()
  endif()
#else()
#  if(WIN32)
#    if(NOT BORLAND)
#      if(NOT CYGWIN)
#        if(NOT MINGW)
#                set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /w")
#                # CMake versions before 2.4 did not put the source-file-specific flags
#                # after the directory-level flags.  Remove optimization flags from the
#                # CMAKE_C_FLAGS* variables so they can be overridden.
#                if("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" LESS 2.4)
#                        foreach(c "" "_DEBUG" "_RELEASE" "_MINSIZEREL" "_RELWITHDEBINFO")
#                                string(REGEX REPLACE
#                                  "-O[^ ]*( |$)" "" CMAKE_C_FLAGS${c} "${CMAKE_C_FLAGS${c}}")
#                        endforeach()
#                endif()
#        endif()
#      endif()
#    endif()
#  endif()
endif()

add_library(otb6S ${LIBF2C_SRCS} ${OTB6S_SRCS} ${OTB6S_MAIN_SRCS})
if(UNIX)
    target_link_libraries(otb6S m)
endif()
if(OTB_LIBRARY_PROPERTIES)
  set_target_properties(otb6S PROPERTIES ${OTB_LIBRARY_PROPERTIES})
endif()

if(NOT OTB_INSTALL_NO_LIBRARIES)
  install(TARGETS otb6S
    RUNTIME DESTINATION ${OTB_INSTALL_BIN_DIR} COMPONENT RuntimeLibraries
    LIBRARY DESTINATION ${OTB_INSTALL_LIB_DIR} COMPONENT RuntimeLibraries
    ARCHIVE DESTINATION ${OTB_INSTALL_LIB_DIR} COMPONENT Development)
endif()

if(NOT OTB_INSTALL_NO_DEVELOPMENT)
  file(GLOB __files1 "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
  install(FILES ${__files1}
    DESTINATION ${OTB_INSTALL_INCLUDE_DIR}/Utilities/otb6S
    COMPONENT Development)
  file(GLOB __files2 "${CMAKE_CURRENT_SOURCE_DIR}//6SV1.0B/*.h")
  install(FILES ${__files2}
    DESTINATION ${OTB_INSTALL_INCLUDE_DIR}/Utilities/otb6S/6SV1.0B
    COMPONENT Development)
endif()


