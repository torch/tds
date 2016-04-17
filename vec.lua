local ffi = require 'ffi'
local tds = require 'tds.env'
local elem = require 'tds.elem'
local C = tds.C

-- vec-independent temporary buffers
local val__ = C.tds_elem_new()
ffi.gc(val__, C.tds_elem_free)

local vec = {}
local NULL = not jit and ffi.C.NULL or nil

local mt = {}

function mt:insert(...)
   local lkey, lval
   if select('#', ...) == 1 then
      lkey, lval = #self+1, select(1, ...)
   elseif select('#', ...) == 2 then
      lkey, lval = select(1, ...), select(2, ...)
   else
      error('[key] value expected')
   end
   assert(self)
   assert(type(lkey) == 'number' and lkey > 0, 'positive number expected as key')
   if lval or type(lval) == 'boolean' then
      elem.set(val__, lval)
   else
      C.tds_elem_set_nil(val__)
   end
   if C.tds_vec_insert(self, lkey-1, val__) == 1 then
      error('out of memory')
   end
end

function mt:remove(lkey)
   lkey = lkey or #self
   assert(self)
   assert(type(lkey) == 'number' and lkey > 0, 'positive number expected as key')
   C.tds_vec_remove(self, lkey-1)
end

function mt:resize(size)
   assert(type(size) == 'number' and size >= 0, 'size must be a positive number')
   C.tds_vec_resize(self, size)
end

if pcall(require, 'torch') then
   function mt:concatstorage(sep, i, j)
      i = i or 1
      j = j or #self
      local sepsize = 0
      if sep then
         sep = torch.CharStorage():string(sep)
         sepsize = sep:size()
      end
      local buffer = torch.CharStorage()
      local size = 0
      for k=i,j do
         local str = tostring(self[k])
         assert(str, 'vector elements must return a non-nil tostring()')
         str = torch.CharStorage():string(str)
         local strsize = str:size()
         if size+strsize+sepsize > buffer:size() then
            buffer:resize(math.max(buffer:size()*1.5, size+strsize+sepsize))
         end
         if sep and size > 0 then
            local view = torch.CharStorage(buffer, size+1, sepsize)
            view:copy(sep)
            size = size + sepsize
         end
         local view = torch.CharStorage(buffer, size+1, strsize)
         view:copy(str)
         size = size + strsize
      end
      buffer:resize(size)
      return buffer
   end

   function mt:concat(sep, i, j)
      return self:concatstorage(sep, i, j):string()
   end
end

function mt:sort(compare)
   if type(compare) == 'function' then
      local function compare__(cval1, cval2)
         local lval1, lval2
         if C.tds_elem_isnil(cval1) == 0 then
            lval1 = elem.get(cval1)
         end
         if C.tds_elem_isnil(cval2) == 0 then
            lval2 = elem.get(cval2)
         end
         return compare(lval1, lval2) and -1 or 1
      end
      local cb_compare__ = ffi.cast('int (*)(tds_elem*, tds_elem*)', compare__)
      C.tds_vec_sort(self, cb_compare__)
      cb_compare__:free()
   else -- you must know what you are doing
      assert(compare ~= nil, 'compare function must be a lua or C function')
      C.tds_vec_sort(self, compare)
   end
end

local function isvec(tbl)
   for k, v in pairs(tbl) do
      if type(k) ~= 'number' then
         return false
      end
   end
   return true
end

local function fill(self, tbl)
   assert(isvec(tbl), 'lua table with number keys expected')
   for key, val in pairs(tbl) do
      if type(val) == 'table' then
         if isvec(val) then
            self[key] = tds.Vec(val)
         else
            self[key] = tds.Hash(val)
         end
      else
         self[key] = val
      end
   end
end

function vec:__new(...) -- beware of the :
   local self = C.tds_vec_new()
   if self == NULL then
      error('unable to allocate vec')
   end
   self = ffi.cast('tds_vec&', self)
   ffi.gc(self, C.tds_vec_free)
   if select('#', ...) == 1 and type(select(1, ...)) == 'table' then
      fill(self, select(1, ...))
   elseif select('#', ...) > 0 then
      fill(self, {...})
   end
   return self
end

function vec:__newindex(lkey, lval)
   assert(self)
   assert(type(lkey) == 'number' and lkey > 0, 'positive number expected as key')
   if lval or type(lval) == 'boolean' then
      elem.set(val__, lval)
   else
      C.tds_elem_set_nil(val__)
   end
   if C.tds_vec_set(self, lkey-1, val__) == 1 then
      error('out of memory')
   end
end

function vec:__index(lkey)
   local lval
   assert(self)
   if type(lkey) == 'number' then
      assert(lkey > 0, 'positive number expected as key')
      C.tds_vec_get(self, lkey-1, val__)
      if C.tds_elem_isnil(val__) == 0 then
         lval = elem.get(val__)
      end
   else
      local method = rawget(mt, lkey)
      if method then
         return method
      else
         error('invalid key (number) or method name')
      end
   end
   return lval
end

function vec:__len()
   assert(self)
   return tonumber(C.tds_vec_size(self))
end

function vec:__ipairs()
   assert(self)
   local k = 0
   return function()
      k = k + 1
      if k <= C.tds_vec_size(self) then
         return k, self[k]
      end
   end
end

vec.__pairs = vec.__ipairs

ffi.metatype('tds_vec', vec)

if pcall(require, 'torch') and torch.metatype then

   function vec:__write(f)
      f:writeLong(#self)
      for k,v in ipairs(self) do
         f:writeObject(v)
      end
   end

   function vec:__read(f)
      local n = f:readLong()
      for k=1,n do
         local v = f:readObject()
         self[k] = v
      end
   end

   vec.__factory = vec.__new
   vec.__version = 0

   torch.metatype('tds.Vec', vec, 'tds_vec&')

end

function vec:__tostring()
   local str = {}
   table.insert(str, string.format('tds.Vec[%d]{', #self))
   for k,v in ipairs(self) do
      local kstr = string.format("%5d : ", tostring(k))
      local vstr = tostring(v) or type(v)
      local sp = string.rep(' ', #kstr)
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
      if k == 20 then
         table.insert(str, '...')
         break
      end
   end
   table.insert(str, '}')
   return table.concat(str, '\n')
end

-- table constructor
local vec_ctr = {}
setmetatable(
   vec_ctr,
   {
      __index = vec,
      __newindex = vec,
      __call = vec.__new
   }
)
tds.vec = vec_ctr
tds.Vec = vec_ctr

return vec_ctr
