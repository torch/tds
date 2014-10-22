local tds = require 'tds.env'
local ffi = require 'ffi'
local C = tds.C

local elem = {}

local elem_ctypes = {}

function elem.addctype(ttype, free_p, setfunc, getfunc)
   elem_ctypes[ttype] = setfunc
   elem_ctypes[torch.pointer(free_p)] = getfunc -- ffi.cast intptr_t?
end

function elem.set(elem, lelem)
   if type(lelem) == 'string' then
      C.tds_elem_set_string(elem, lelem, #lelem)
   elseif type(lelem) == 'number' then
      C.tds_elem_set_number(elem, lelem)
   else
      local tname = torch.typename(lelem)
      local setfunc = tname and elem_ctypes[tname]
      if setfunc then
         C.tds_elem_set_pointer(elem, setfunc(lelem))
      else
         error('unsupported key/value type (set)')
      end
   end
end

function elem.get(elem)
   assert(elem)
   local elemtype = C.tds_elem_type(elem)
   if elemtype == 110 then--string.byte('n') then
      local value =  C.tds_elem_get_number(elem)
      return value
   elseif elemtype == 115 then--string.byte('s') then
      local value = ffi.string(C.tds_elem_get_string(elem), C.tds_elem_get_string_size(elem))
      return value
   elseif elemtype == 112 then--string.byte('p') then
      local lelem_p = C.tds_elem_get_pointer(elem)
      local free_p = C.tds_elem_get_pointer_free(elem)
      local getfunc = elem_ctypes[torch.pointer(free_p)]
      if getfunc then
         local value = getfunc(lelem_p)
         return value
      else
         error('unsupported key/value type (get)')
      end
   else
      error('unsupported key/value type (get)')
   end
end

-- torch specific
if pcall(require, 'torch') then
   local T = ffi.C

   for _, Real in ipairs{'Double', 'Float', 'Long', 'Int', 'Short', 'Char', 'Byte'} do
      local cdefs = [[
void THRealTensor_retain(THRealTensor *self);
void THRealTensor_free(THRealTensor *self);
]]
      cdefs = cdefs:gsub('Real', Real)
      ffi.cdef(cdefs)

      local THTensor_retain = T[string.format('TH%sTensor_retain', Real)]
      local THTensor_free = T[string.format('TH%sTensor_free', Real)]
      local tensor_type_id = string.format('torch.%sTensor', Real)
      elem.addctype(
         string.format('torch.%sTensor', Real),
         THTensor_free,
         function(lelem)
            local lelem_p = lelem:cdata()
            THTensor_retain(lelem_p)
            return lelem_p,  THTensor_free
         end,
         function(lelem_p)
            THTensor_retain(lelem_p)
            local value = torch.pushudata(lelem_p, tensor_type_id)
            return value
         end
      )

   end
end

return elem
