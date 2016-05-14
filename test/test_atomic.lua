local tds = require 'tds'

local a = tds.AtomicCounter()

print(a)
print(a:get())
assert(a:get()==0)
print(a:inc())
assert(a:get()==1)
print(a:inc())
assert(a:get()==2)
print(a)
a:set(100)
print(a:get())
assert(a:get()==100)
print(a)

print('OK')
