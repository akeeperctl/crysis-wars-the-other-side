function table.dump(tbl, indent)
    indent = indent or ""
    local result = "{"
    local first = true
    for k, v in pairs(tbl) do
      if not first then
        result = result .. ","
      end
      first = false
      if type(v) == "table" then
        result = result .. "\n" .. indent .. tostring(k) .. ": " .. table.dump(v, indent .. "  ")
      else
        result = result .. "\n" .. indent .. tostring(k) .. ": " .. tostring(v)
      end
    end
    result = result .. "\n" .. indent .. "}"
    return result
end