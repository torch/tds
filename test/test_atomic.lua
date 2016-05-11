local tds = require 'tds'

local a = tds.AtomicCounter()

print(a)
print(a:get())
print(a:inc())
print(a:inc())
print(a)
a:set(100)
print(a:get())
print(a)