local ffi = require 'ffi'
local tds = require 'tds.env'
local C = tds.C

local hash = {}

function tds.hash()
   local self = C.tds_hash_new()
   if self == nil then
      error('unable to allocate hash')
   end
   ffi.gc(self, C.tds_hash_free)
   return self
end

local function lua2Celem(lelem)
   local elem
   if type(lelem) == 'string' then
      elem = C.tds_elem_string_new(lelem, #lelem)
   elseif type(lelem) == 'number' then
      elem = C.tds_elem_number_new(lelem)
   else
      error('string or number key/value expected')
   end
   if elem == nil then
      error('unable to allocate C key/value')
   end
   return elem
end

local function lkey2obj(self, lkey)
   local obj
   if type(lkey) == 'string' then
      obj = C.tds_hash_search_string(self, lkey, #lkey)
   elseif type(lkey) == 'number' then
      obj = C.tds_hash_search_number(self, lkey)
   else
      error('string or number key expected')
   end
   return obj
end

function hash:__newindex(lkey, lval)
   local obj = lkey2obj(self, lkey)
   if obj ~= nil then
      if lval then
         local val = lua2Celem(lval)
         C.tds_hash_object_set_value(obj, val)
      else
         C.tds_hash_remove(self, obj)
      end
   else
      local key = lua2Celem(lkey)
      local val = lua2Celem(lval)
      local obj = C.tds_hash_object_new(key, val)
      C.tds_hash_insert(self, obj)
   end
end

function hash:__index(lkey)
   local obj = lkey2obj(self, lkey)
   if obj ~= nil then
      local val = C.tds_hash_object_value(obj)
      local valtyp = ffi.string(C.tds_elem_typename(val))
      if valtyp == 'number' then
         return ffi.cast('tds_number*', val).value
      elseif valtyp == 'string' then
         return ffi.string(ffi.cast('tds_string*', val).data)
      else
         error(string.format('value type <%s> not supported yet', valtyp))
      end
   end
end

function hash:__len()
   return tonumber(C.tds_hash_size(self))
end

ffi.metatype('tds_hash', hash)

return tds.hash
