local ffi = require 'ffi'
local tds = require 'tds.env'
local elem = require 'tds.elem'
local C = tds.C

local hash = {}

function hash.__new()
   local self = C.tds_hash_new()
   if self == nil then
      error('unable to allocate hash')
   end
   self = ffi.cast('tds_hash&', self)
   ffi.gc(self, C.tds_hash_free)
   return self
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
         C.tds_elem_free_content(val)
         elem.set(val, lval)
      else
         C.tds_hash_remove(self, obj)
         C.tds_hash_object_free(obj)
      end
   else
      if lval then
         local obj = C.tds_hash_object_new()
         local key = C.tds_hash_object_key(obj)
         local val = C.tds_hash_object_value(obj)
         elem.set(val, lval)
         elem.set(key, lkey)
         C.tds_hash_insert(self, obj)
      end
   end
end

function hash:__index(lkey)
   assert(self)
   local obj = findkey(self, lkey)
   if obj ~= nil then
      local val = elem.get(C.tds_hash_object_value(obj))
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
         local key = elem.get(C.tds_hash_object_key(obj))
         local val = elem.get(C.tds_hash_object_value(obj))
         return key, val
      end
   end
end

ffi.metatype('tds_hash', hash)

if pcall(require, 'torch') and torch.metatype then

   function hash:__write(f)
      f:writeLong(#self)
      for k,v in pairs(self) do
         f:writeObject(k)
         f:writeObject(v)
      end
   end

   function hash:__read(f)
      local n = f:readLong()
      for i=1,n do
         local k = f:readObject()
         local v = f:readObject()
         self[k] = v
      end
   end

   hash.__factory = hash.__new
   hash.__version = 0

   torch.metatype('tds_hash', hash, 'tds_hash&')

end

-- table constructor
local hash_ctr = {}
setmetatable(
   hash_ctr,
   {
      __index = hash,
      __newindex = hash,
      __call = hash.__new
   }
)
tds.hash = hash_ctr

return hash_ctr
