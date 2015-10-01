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

function hash:__tostring()
   local str = {}
   table.insert(str, string.format('tds_hash[%d]{', #self))
   local function key2str(k)
      if type(k) == 'string' or type(k) == 'number' then
         return tostring(k)
      elseif torch then
         return torch.type(k)
      else
         return type(k)
      end
   end
   local ksz = 0
   local idx = 0
   for k,v in pairs(self) do
      ksz = math.max(ksz, #key2str(k))
      idx = idx + 1
      if idx == 20 then
         break
      end
   end

   idx = 0
   for k,v in pairs(self) do
      local kstr = key2str(k)
      kstr = string.format("%s%s : ", kstr, string.rep(' ', ksz-#kstr))
      local vstr = tostring(v) or type(v)
      local sp = string.rep(' ', ksz+3)
      local i = 0
      vstr = vstr:gsub(
         '([^\n]+)',
         function(line)
            i = i + 1
            if i == 1 then
               return kstr .. line
            else
               return sp .. line
            end
         end
      )
      table.insert(str, vstr)
      idx = idx + 1
      if idx == 20 then
         table.insert(str, '...')
         break
      end
   end
   table.insert(str, '}')
   return table.concat(str, '\n')
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
