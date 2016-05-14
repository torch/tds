local ffi = require 'ffi'
local tds = require 'tds.env'
local C = tds.C

local mt = {}

local atomic = {}
local NULL = not jit and ffi.C.NULL or nil

function mt:inc()
  return tonumber(C.tds_atomic_inc(self))
end

function mt:get()
  return tonumber(C.tds_atomic_get(self))
end

function mt:set(value)
  return C.tds_atomic_set(self, value)
end

function atomic:__new(...)
  if C.tds_has_atomic()==0 then
    error('atomic counter not available (Torch not found)')
  end
  local self = C.tds_atomic_new()
  if self == NULL then
    error('unable to allocate atomic')
  end
  self = ffi.cast('tds_atomic_counter&', self)
  ffi.gc(self, C.tds_atomic_free)
  return self
end

function atomic:__tostring()
  return 'tds.AtomicCounter='.. tonumber(C.tds_atomic_get(self))
end

function atomic:__index(lkey)
  local method = rawget(mt, lkey)
  if method then
    return method
  else
    error('no such method')
  end
end

ffi.metatype('tds_atomic_counter', atomic)


if pcall(require, 'torch') and torch.metatype then

   function atomic:__write(f)
      f:writeLong(self:get())
   end

   function atomic:__read(f)
      self:set(f:readLong())
   end

   atomic.__factory = atomic.__new
   atomic.__version = 0

   torch.metatype('tds.AtomicCounter', atomic, 'tds_atomic_counter&')

end


local atomic_ctr = {}
setmetatable(
  atomic_ctr,
  {
    __call = atomic.__new
  }
)

tds.atomic_counter = atomic_ctr
tds.AtomicCounter = atomic_ctr

return atomic_ctr