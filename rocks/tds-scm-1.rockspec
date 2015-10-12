package = "tds"
version = "scm-1"

source = {
   url = "git://github.com/torch/tds.git"
}

description = {
   summary = "Standard C data structures for LuaJIT",
   detailed = [[
   ]],
   homepage = "https://github.com/torch/tds",
   license = "BSD"
}

dependencies = {
   "lua >= 5.1",
}

build = {
   type = "cmake",
   variables = {
      CMAKE_BUILD_TYPE="Release",
      CMAKE_PREFIX_PATH="$(LUA_BINDIR)/..",
      LUA_PATH="$(LUADIR)",
      LUA_CPATH="$(LIBDIR)"
   }
}
