local ffi = require 'ffi'
local tds = require 'tds.env'
local C = tds.C

local hash = {}

function hash.new()
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
   assert(self)
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

local function objelem2lua(val)
   assert(val)
   local valtyp = ffi.string(C.tds_elem_typename(val))
   if valtyp == 'number' then
      return ffi.cast('tds_number*', val).value
   elseif valtyp == 'string' then
      val = ffi.cast('tds_string*', val)
      return ffi.string(val.data, val.size)
   else
      error(string.format('value type <%s> not supported yet', valtyp))
   end
end

function hash:__index(lkey)
   assert(self)
   local obj = lkey2obj(self, lkey)
   if obj ~= nil then
      local val = C.tds_hash_object_value(obj)
      return objelem2lua(val)
   end
end

function hash:__len()
   assert(self)
   return tonumber(C.tds_hash_size(self))
end

function hash:__pairs()
   assert(self)
   local iterator = C.tds_hash_iterator_new(self)
   ffi.gc(iterator, C.tds_hash_iterator_free)

   return function()
      local obj = C.tds_hash_iterator_next(iterator)
      if obj ~= nil then
         local key = C.tds_hash_object_key(obj)
         local val = C.tds_hash_object_value(obj)
         return objelem2lua(key), objelem2lua(val)
      end
   end
end

hash.pairs = hash.__pairs

ffi.metatype('tds_hash', hash)

-- table constructor
local hash_ctr = {}
setmetatable(
   hash_ctr,
   {
      __index = hash,
      __newindex = hash,
      __call = hash.new
   }
)
tds.hash = hash_ctr

return hash_ctr
