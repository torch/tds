local tds = require 'tds.env'
local ffi = require 'ffi'
local C = tds.C

local elem = {}

local elem_ctypes = {}
local elem_ctypes_abbr2name = {}
local elem_ctypes_name2abbr = {}

function elem.type()
end

function elem.addctype(ttype, free_p, setfunc, getfunc, abbr)
   elem_ctypes[ttype] = setfunc
   elem_ctypes[tonumber(ffi.cast('intptr_t', free_p))] = getfunc
   if abbr then
     elem_ctypes_abbr2name[abbr] = ttype
     elem_ctypes_name2abbr[ttype] = abbr
   end
end

function elem.set(celem, lelem)
   if type(lelem) == 'string' then
      C.tds_elem_set_string(celem, lelem, #lelem)
   elseif type(lelem) == 'number' then
      C.tds_elem_set_number(celem, lelem)
   elseif type(lelem) == 'boolean' then
      C.tds_elem_set_boolean(celem, lelem)
   else
      local tname = elem.type(lelem)
      local setfunc = tname and elem_ctypes[tname]
      if setfunc then
         C.tds_elem_set_pointer(celem, setfunc(lelem))
      else
         error(string.format('unsupported key/value type <%s> (set)', tname and tname or type(lelem)))
      end
      local abbr = elem_ctypes_name2abbr[tname]
      if abbr then
        C.tds_elem_set_subtype(celem, abbr)
      end
   end
end

function elem.get(celem)
   assert(celem)
   local elemtype = C.tds_elem_type(celem)
   if elemtype == 110 then--string.byte('n') then
      local value =  C.tds_elem_get_number(celem)
      return value
   elseif elemtype == 98 then--string.byte('b') then
      local value =  C.tds_elem_get_boolean(celem)
      return value
   elseif elemtype == 115 then--string.byte('s') then
      local value = ffi.string(C.tds_elem_get_string(celem), tonumber(C.tds_elem_get_string_size(celem)))
      return value
   elseif elemtype == 112 then--string.byte('p') then
      local subtype = C.tds_elem_subtype(celem)
      local lelem_p = C.tds_elem_get_pointer(celem)
      local free_p = C.tds_elem_get_pointer_free(celem)
      local getfunc = elem_ctypes[tonumber(ffi.cast('intptr_t', free_p))]
      if getfunc then
         local value = getfunc(lelem_p, subtype)
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
   local T = ffi.os ~= 'Windows' and ffi.C or ffi.load('TH')

   elem.type = torch.typename

   -- even though this one looks general, one need a type check system to
   -- make it work properly: for converting a cdata to its typename.
   -- one way would be to use ffi.istype()
   elem.addctype(
      'tds.Hash',
      C.tds_hash_free,
      function(lelem)
         C.tds_hash_retain(lelem)
         return lelem,  C.tds_hash_free
      end,
      function(lelem_p)
         local lelem = ffi.cast('tds_hash&', lelem_p)
         C.tds_hash_retain(lelem)
         ffi.gc(lelem, C.tds_hash_free)
         return lelem
      end
   )

   elem.addctype(
      'tds.Vec',
      C.tds_vec_free,
      function(lelem)
         C.tds_vec_retain(lelem)
         return lelem,  C.tds_vec_free
      end,
      function(lelem_p)
         local lelem = ffi.cast('tds_vec&', lelem_p)
         C.tds_vec_retain(lelem)
         ffi.gc(lelem, C.tds_vec_free)
         return lelem
      end
   )

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
         tensor_type_id,
         THTensor_free,
         function(lelem)
            local lelem_p = lelem:cdata()
            THTensor_retain(lelem_p)
            return lelem_p,  THTensor_free
         end,
         function(lelem_p, subtype)
            THTensor_retain(lelem_p)
            local tensor_type_id = elem_ctypes_abbr2name[subtype]
            local lelem = torch.pushudata(lelem_p, tensor_type_id)
            return lelem
         end,
         string.byte(Real, 1)
      )
   end
end

return elem
