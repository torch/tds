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

local function setelem(elem, lelem)
   if type(lelem) == 'string' then
      C.tds_elem_set_string(elem, lelem, #lelem)
   elseif type(lelem) == 'number' then
      C.tds_elem_set_number(elem, lelem)
   else
      error('string or number key/value expected')
   end
end

local function findkey(self, lkey)
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
   local obj = findkey(self, lkey)
   if obj ~= nil then
      if lval then
         local val = C.tds_hash_object_value(obj)
         setelem(val, lval)
      else
         C.tds_hash_remove(self, obj)
         C.tds_hash_object_free(obj)
      end
   else
      local obj = C.tds_hash_object_new()
      local key = C.tds_hash_object_key(obj)
      local val = C.tds_hash_object_value(obj)
      setelem(val, lval)
      setelem(key, lkey)
      C.tds_hash_insert(self, obj)
   end
end

local function getelem(elem)
   assert(elem)
   local value
   local elemtype = C.tds_elem_type(elem)
   if elemtype == 110 then--string.byte('n') then
      value =  C.tds_elem_get_number(elem)
   elseif elemtype == 115 then--string.byte('s') then
      value = ffi.string(C.tds_elem_get_string(elem), C.tds_elem_get_string_size(elem))
   else
      error(string.format('value type <%s> not supported yet', elemtype))
   end
   return value
end

function hash:__index(lkey)
   assert(self)
   local obj = findkey(self, lkey)
   if obj ~= nil then
      local val = getelem(C.tds_hash_object_value(obj))
      return val
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
         local key = getelem(C.tds_hash_object_key(obj))
         local val = getelem(C.tds_hash_object_value(obj))
         return key, val
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
