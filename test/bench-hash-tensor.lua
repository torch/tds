local tds = require 'tds'

local N = 1000000
local M = 500

torch.manualSeed(1111)

local h
if arg[1] == 'lua' then
   print('using lua tables!')
   h = {}
else
   print('using tds tables!')
   h = tds.hash()
end

local perm = torch.randperm(N)

for i=1,N do
   h[perm[i]] = torch.rand(M)
end

for i=1,N do
   h[torch.rand(M)] = perm[i]
end

for i=N/4,N/2 do
   h[perm[i]] = nil
end

local sum = 0
for k,v in pairs(h) do
   if type(k) == 'number' then
      sum = sum + v:sum()
   else
      sum = sum + k:sum()
   end
end

print('SUM', sum)
