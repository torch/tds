cmake_minimum_required (VERSION 2.8)
cmake_policy(VERSION 2.8)

set(PKGNAME tds)

add_library(${PKGNAME} MODULE
  "tds_utils.c"
  "tds_elem.c"
  "tds_hash.c"
  "tds_vec.c"
)

install(TARGETS ${PKGNAME} LIBRARY
  DESTINATION ${LUA_CPATH})

FILE(GLOB luafiles *.lua)

install(FILES ${luafiles}
  DESTINATION ${LUA_PATH}/${PKGNAME})
