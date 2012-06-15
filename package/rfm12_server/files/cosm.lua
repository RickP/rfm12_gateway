re("luacurl")

function process (input)

  parts = split(input, ',')

  c = curl.new()
  c:setopt(curl.OPT_URL, "http://www.pachube.com/api/feeds/63435/datastreams/" .. parts[1] .. ".csv")
  c:setopt(curl.OPT_CUSTOMREQUEST, "PUT")
  c:setopt(curl.OPT_HTTPHEADER, "X-PachubeApiKey: R0wnmPECxJZzuqfuAIzxBBtZpfCSAKw2THB6TFg2R3Z6QT0g")
  c:setopt(curl.OPT_POSTFIELDS, parts[2])
  c:perform()

  return "OK"
end

function split(str, pat)
   local t = {}
   local fpat = "(.-)" .. pat
   local last_end = 1
   local s, e, cap = str:find(fpat, 1)
   while s do
      if s ~= 1 or cap ~= "" then
	 table.insert(t,cap)
      end
      last_end = e+1
      s, e, cap = str:find(fpat, last_end)
   end
   if last_end <= #str then
      cap = str:sub(last_end)
      table.insert(t, cap)
   end
   return t
end

