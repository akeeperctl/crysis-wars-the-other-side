--************************************************************************* 
 --AlienKeeper Source File.
 --Copyright (C), AlienKeeper, 2024.
--*************************************************************************

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

function table.safedump(data)
	if (data and type(data) == "table") then
		return table.dump(data)
	else
		return "{}"
	end
end

function table.copy(tbl_source, tbl_dest)
  for k, v in pairs(tbl_source) do
    tbl_dest[k] = v
  end
end
