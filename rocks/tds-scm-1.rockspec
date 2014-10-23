package = "tds"
version = "scm-1"

source = {
   url = "git://github.com/torch/tds-ffi.git"
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
   type = "builtin",
   modules = {
      ["tds.env"] = "env.lua",
      ["tds.init"] = "init.lua",
      ["tds.cdefs"] = "cdefs.lua",
      ["tds.hash"] = "hash.lua",
      ["tds.elem"] = "elem.lua",
      libtds = {
         sources = {
            "tds_utils.c",
            "tds_elem.c",
            "tds_hash.c",
            "tommyds/tommyds/tommyhashlin.c",
            "tommyds/tommyds/tommylist.c",
            "tommyds/tommyds/tommyhash.c",
            "tommyds/tommyds/tommyarrayof.c"
         },
         incdirs = {
            "tommyds/tommyds",
         }
      }
   }
}
