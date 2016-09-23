cmake_minimum_required (VERSION 2.8)
cmake_policy(VERSION 2.8)

FIND_PACKAGE(Torch QUIET)
IF(Torch_FOUND)
  ADD_DEFINITIONS(-DHAS_TORCH)
ENDIF()

set(PKGNAME tds)

add_library(${PKGNAME} MODULE
  "tds_utils.c"
  "tds_elem.c"
  "tds_hash.c"
  "tds_vec.c"
  "tds_atomic_counter.c"
)
SET_TARGET_PROPERTIES(${PKGNAME} PROPERTIES
  PREFIX "lib"
  IMPORT_PREFIX "lib")

IF(Torch_FOUND)
  TARGET_LINK_LIBRARIES(${PKGNAME} TH)
ENDIF()

install(TARGETS ${PKGNAME} LIBRARY
  DESTINATION ${LUA_CPATH})

FILE(GLOB luafiles *.lua)

install(FILES ${luafiles}
  DESTINATION ${LUA_PATH}/${PKGNAME})

if(MSVC)
    set(CMAKE_MODULE_LINKER_FLAGS
        "${CMAKE_MODULE_LINKER_FLAGS} /DEF:..\\tds.def")
endif()
