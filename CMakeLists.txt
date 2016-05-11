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

IF(Torch_FOUND)
  TARGET_LINK_LIBRARIES(${PKGNAME} TH)
ENDIF()

install(TARGETS ${PKGNAME} LIBRARY
  DESTINATION ${LUA_CPATH})

FILE(GLOB luafiles *.lua)

install(FILES ${luafiles}
  DESTINATION ${LUA_PATH}/${PKGNAME})
