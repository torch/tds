local ffi = require 'ffi'

local tds = {}

tds.C = ffi.load(package.searchpath('libtds', package.cpath))

return tds
