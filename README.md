Data structures which do not rely on Lua memory allocator, nor being
limited by Lua garbage collector.

Under the hood, this is a LuaJIT FFI interface to Tommy DS.

## Example

```Lua
local tds = require 'tds'

-------------------------------------------------
-- BASIC
-------------------------------------------------

local h = tds.hash()

-- set
h.foo = "bar"
h["scm-1"] = "git"

-- the key can be a number
h[1] = "hey"
-- the value can be a number too
h.count = 1234

-- get
print(h.foo)      -- "bar"
print(h["scm-1"]) -- "git"
print(h.count)    -- 1234

-- length
print(#h)         -- 4

-- iterator
for k,v in pairs(h) do
    print(k, v)
end

-- unset
h.foo = nil
print(h.foo)      -- nil

-------------------------------------------------
-- ADVANCED
-------------------------------------------------

-- you can nest hashes, i.e use another hash as value
local misc = tds.hash()
misc.hello = "world"

h.baz = misc
print(h.baz.hello) -- "world"

if pcall(require, 'torch') then
    -- Torch7

    -- tds plays nice with Torch since you can set a tensor as value
    h.weights = torch.randn(3, 2)
    print(h.weights) -- (...) [torch.DoubleTensor of dimension 3x2]

    -- you can also serialize/unserialize with Torch utils
    local f = torch.MemoryFile("rw"):binary()

    -- serialize
    f:writeObject(h)

    -- unserialize
    f:seek(1)
    local clone = f:readObject()

    assert(#h == #clone)

    f:close()

    -- Note: you can also use the high level interface to save on disk:
    torch.save("dump.bin", h)
end
```
