Data structures which do not rely on Lua memory allocator, nor being
limited by Lua garbage collector.

Only C types can be stored: supported types are currently number, strings,
the data structures themselves (see [nesting](#tds.nesting): e.g. it is
possible to have a Hash containing a Hash or a Vec), and torch tensors and
storages. All data structures can store heterogeneous objects, and support
[torch serialization](#tds.serialization).

It is easy to extend the support to [other C types](#tds.extend).

Note that `tds` relies currently on FFI, and works both with
[luajit](http://www.luajit.org) or [Lua 5.2](http://www.lua.org), provided
the latter is installed with
[luaffi](https://github.com/facebook/luaffifb). The dependency on FFI will
be removed in the future.

<a name="tds.Hash"/>
## d = tds.Hash([tbl]) ##

Creates a hash table which implements the lua operators `[key]`, `#`
and `pairs()`, and in very similar way than lua tables.

A hash can contain any element (either as key or value) supported by `tds`.

If a lua table `tbl` is provided, the Hash will be filled up with
corresponding elements. Tables inside the `tbl` will be also converted
(recursively) to [tds.Vec](#tds.Vec) (if they contain only number keys) or
[tds.Hash](#tds.Hash) otherwise.

### d[key] = value ###

Store the given (`key`, `value`) pair in the hash table. If `value` is
`nil`, remove the `key` if it exists.

### d[key] ###

Returns the `value` at the given `key`, and `nil` if the `key` does not exist in the hash table.

### #d ###

Returns the number of key-value pairs in the hash table. Note that this acts different than lua tables, the latter
returning the number of elements stored in numbered indices starting from 1.

### pairs(d) ###

Returns an iterator over the hash table `d`. The iterator returns a
key-value pair at each step, or nil if reaching the end.  Typical usage
will be:
```lua
for k,v in pairs(d) do
  -- <do something>
end
```

Note: as for Lua standard tables, the iterator behavior is undefined if a
new key is inserted in the hash while iterating. Modifying existing keys is
however allowed.

<a name="tds.Vec"/>
## d = tds.Vec([... || tbl]) ##

Creates a vector of elements indexed by numbers starting from 1. If a
single lua table `tbl` (or several arguments) is (are) passed at
construction, the vector will be filled with the lua table contents (or the
given arguments).

If provided, `tbl` must contain only number keys. Tables inside the `tbl`
(or passed as arguments) will also be converted (recursively) to
[tds.Vec](#tds.Vec) (if they contain only number keys) or
[tds.Hash](#tds.Hash) otherwise.

A vector can contain any element (as value) supported by `tds`, as well as
the `nil` value.

### d[index] = value ###

Store the given `value` at the given `index` (which must be a positive
number). If the index is larger than the current size of the vector, the
vector will be automatically resized. `value` may be `nil`.

### d[index] ###

Returns the `value` at the given `index` or `nil` if it does not exist.

### #d ###

Returns the current size of the vector (note that it includes `nil` values, which are not treated as holes!).

### d:resize(size) ###

Resize the current vector to the given size. If the size is larger than the current size, the vector will be filled with `nil` values.

### d:insert([index], value) ###

Insert `value` in the vector, at position `index`, shifting up all elements
above `index`. If `index` is not provided, insert the element at the end of
the vector.

### d:remove([index]) ###

Remove the element at position `index`, shifting down all elements above
`index`. If `index` is not provided, remove the last element of the vector.

### d:sort(compare) ###

Sort the vector in-place, according to the given `compare` function.

Compare can be either a lua function or a C function.

In the lua case, `compare` is a function which takes two vector elements,
and returns true when the first is less than the second.

If the C case, `compare` must be a FFI type `int (*compare)(const tds_elem *, const tds_elem *)`.
It must return an integer less than, equal to, or greater than zero if the
first argument is considered to be respectively less than, equal to, or
greater than the second. See the include file `tds_elem.h` for more details
about the `tds_elem` structure.

Note that having `compare` as a lua function will lead to a (relatively)
slow sort: elements of the vector will need to be moved in the lua userland
(and thus handled by the GC) in order to be compared.

In the FFI case, `compare` might be a FFI callback, but will also lead to a
slow sort, FFI callbacks being slow.  Fastest speed are obtained when
`compare` is a true compiled C function loaded through FFI.

<a name="tds.Vec.concat"/>
### d:concat([sep, i, j]) ###

Concat all vector elements into a single string. Fails if an element cannot
be converted via `tostring()`.

`sep` is an optional separator string inserted between each elements.

`i` and `j` define an optional range (by default `i=1` and `j` is the size
of the vector).

### d:concatstorage([sep, i, j]) ###

As [concat()](#tds.Vec.concat), but returns a `torch.CharStorage()` instead.

### ipairs(d) ###

Returns an iterator over the vector `d`. The iterator returns a index-value
pair at each step, or nil if reaching the end.  Typical usage will be:
```lua
for i,v in pairs(d) do
  -- <do something>
end
```

### pairs(d) ###

Alias for ipairs(d).

<a name="tds.serialization"/>
## Serialization ##

All `tds` data structures support torch serialization. Example:

```lua
tds = require 'tds'
require 'torch'

-- create a vector containing heterogeneous data
d = tds.Vec(4, 5, torch.rand(3), nil, "hello world")

-- serialize in a buffer
f = torch.MemoryFile("rw")
f:writeObject(d)

-- unserialize
f:seek(1)
print(f:readObject())
```

The example will output:
```
tds.Vec[5]{
    1 : 4
    2 : 5
    3 :  0.1665
         0.8750
         0.7525
        [torch.DoubleTensor of size 3]

    4 : nil
    5 : hello world
}
```

<a name="tds.nesting"/>
## Nesting ##

Nesting is supported in `tds`. However, __reference loops are prohibited__, and
will lead to leaks if used.

Example:

```lua

tds = require 'tds'
require 'torch'

-- create a vector containing heterogeneous data
d = tds.Vec(4, 5, torch.rand(3), tds.Hash(), "hello world")

-- fill up the hash table:
d[4].foo = "bar"
d[4][6] = torch.rand(3)
d[4].stuff = tds.Vec("how", "are", "you", "doing")

print(d)
```

This example will output:
```
tds.Vec[5]{
    1 : 4
    2 : 5
    3 :  0.1958
         0.5663
         0.2777
        [torch.DoubleTensor of size 3]

    4 : tds.Hash[3]{
        foo   : bar
        6     :  0.0105
                 0.7496
                 0.5241
                [torch.DoubleTensor of size 3]

        stuff : tds.Vec[4]{
                    1 : how
                    2 : are
                    3 : you
                    4 : doing
                }
        }
    5 : hello world
}
```

<a name="tds.extend"/>
## Extending to other C types ##

`tds` provides a way to extend to your own C types using the submodule
`tds.elem`:

```lua
local elem = require 'tds.elem'
```

### elem.type(obj) ###

`tds` typechecking is achieved using this function. You can override it for
your own purposes. If torch is detected, `tds` will set `elem.type` to
`torch.typename()`, so in general (if you are using torch!) you should not
worry about this part.

### elem.addctype(ttype, free_p, setfunc, getfunc) ###

Add a new C type into `tds`:
  - `ttype` must be the typename understood by the current `elem.type()` function.
  - `free_p` is a C FFI pointer to a destructor of the C object.
  - `setfunc(luaobj)` takes a __lua object__ and returns a FFI C pointer on this object, as well as a FFI function `free_p` to free this object.
  - `getfunc(cpointer)` takes a __C FFI pointer__ and returns a lua object of the corresponding object.

One must be careful to handle properly reference counting and garbage collection in `setfunc()` and `getfunc()`:
  - `setfunc()` will convert a lua object into a C pointer which will be
    stored into the data structure: the reference count on this object must
    be increased. When removed from the data structure, `tds` will call the
    given `free_p()` function.
  - `getfunc()` will convert a C pointer and push it into lua memory space:
    one must again increase properly the reference count on this object,
    and make sure lua will garbage collect it properly.

Here is a typical example showing how support for `tds.Hash` elements is supported:

```lua
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
```
