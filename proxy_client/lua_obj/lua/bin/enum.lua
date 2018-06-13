local ipairs = ipairs;
local type = type;
local assert = assert;
module(...);

function IsTable(tbl)
	if type(tbl) == "table" then
		return true;
	else
		return false;
	end
end

function CreatEnumTable(tbl, index) 
    assert(IsTable(tbl))
    local enumtbl = {};
    local enumindex = index or 0;

    for i, v in ipairs(tbl) do 
        enumtbl[v] = enumindex + i; 
    end

    return enumtbl;
end