local ffi = require 'ffi'
local tds = require 'tds.env'
local elem = require 'tds.elem'
local C = tds.C

-- hash-independent temporary buffers
local key__ = C.tds_elem_new()
local val__ = C.tds_elem_new()
ffi.gc(key__, C.tds_elem_free)
ffi.gc(val__, C.tds_elem_free)

local hash = {}
local NULL = not jit and ffi.C.NULL or nil

function hash.__new()
   local self = C.tds_hash_new()
   if self == NULL then
      error('unable to allocate hash')
   end
   self = ffi.cast('tds_hash&', self)
   ffi.gc(self, C.tds_hash_free)
   return self
end

function hash:__newindex(lkey, lval)
   assert(self)
   assert(lkey, 'hash index is nil')
   elem.set(key__, lkey)
   if lval then
      elem.set(val__, lval)
   end
   C.tds_hash_insert(self, key__, lval and val__ or NULL)
end

function hash:__index(lkey)
   local lval
   assert(self)
   assert(lkey, 'hash index is nil')
   elem.set(key__, lkey)
   if C.tds_hash_search(self, key__, val__) == 0 then
      lval = elem.get(val__)
   end
   return lval
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
      if C.tds_hash_iterator_next(iterator, key__, val__) == 0 then
         local lkey = elem.get(key__)
         local lval = elem.get(val__)
         return lkey, lval
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
