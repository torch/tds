cmake_minimum_required (VERSION 2.8)
cmake_policy(VERSION 2.8)

set(PKGNAME tds)

include_directories(tommyds/tommyds)

add_library(${PKGNAME} MODULE
  "tds_utils.c"
  "tds_elem.c"
  "tds_hash.c"
  "tommyds/tommyds/tommyhashlin.c"
  "tommyds/tommyds/tommylist.c"
  "tommyds/tommyds/tommyhash.c"
  "tommyds/tommyds/tommyarrayof.c"
)

install(TARGETS ${PKGNAME} LIBRARY
  DESTINATION ${LUA_CPATH})

FILE(GLOB luafiles *.lua)

install(FILES ${luafiles}
  DESTINATION ${LUA_PATH}/${PKGNAME})
