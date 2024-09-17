function table.dump_table(tbl, indent)
    indent = indent or ""
    local result = "{"
    local first = true
    for k, v in pairs(tbl) do
      if not first then
        result = result .. ","
      end
      first = false
      if type(v) == "table" then
        result = result .. "\n" .. indent .. tostring(k) .. ": " .. dump_table(v, indent .. "  ")
      else
        result = result .. "\n" .. indent .. tostring(k) .. ": " .. tostring(v)
      end
    end
    result = result .. "\n" .. indent .. "}"
    return result
end