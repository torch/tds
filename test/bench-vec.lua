local tds = require 'tds'
local N = 10000000

require 'torch'
torch.manualSeed(1111)
local timer = torch.Timer()

local perm = torch.randperm(N)

local tbl

if arg[1] == 'lua' then
   print('using lua tables!')
   tbl = {}
else
   print('using tds tables!')
   tbl = tds.Vec()
end

print('filling up')
timer:reset()
for i=1,N do
   tbl[i] = perm[i]
end
print('  time:', timer:time().real)
print()

print('done, now summing')
timer:reset()
local perm = torch.randperm(N)
local sum = 0
for i=1,N do
   sum = sum + tbl[perm[i]]
end
print(sum, #tbl)
print('  time:', timer:time().real)
print()

print('sorting!')
timer:reset()
if arg[1] == 'lua' then
   table.sort(
      tbl,
      function(a, b)
         return a < b
      end
   )
else
   tbl:sort(
      function(a, b)
         return a < b
      end
   )
end
print('  time:', timer:time().real)
print()

print('now run over it')
timer:reset()
local n = 0
for key, value in ipairs(tbl) do
   if n < 10 then
      print(key, value)
   end
   n = n + 1
end
print()
print('size', n)
print('  time:', timer:time().real)
print()
