import os
import pandas as pd
import config
def GenLuaType(type_str):
    if type_str == "INT":
        return "number"
    if type_str == "STRING":
        return "string"
    if type_str == "BOOL":
        return "boolean"
    if type_str == "FLOAT":
        return "number"
    

def handle_excelfile_tolua(filename):
    print("GenerateTableData.py handle_excelfile_tolua", filename)
    file = pd.read_excel(filename)
    filename = os.path.splitext(os.path.basename(filename))[0]
    tabDataName = "Table" + filename + "Data"
    tabDataManagerName = tabDataName + "Manager"

    headers = file.iloc[0]
    attrname = file.columns.tolist()
    lua_str = "---{}\n\n".format(config.WARNING_NOTE)
    lua_str =lua_str + "---@class " + tabDataName + "\n"

    for i in range(headers.values.size):
        lua_str = lua_str + "---@field "+ attrname[i] + " " + GenLuaType(headers.values[i]) + "\n"
    lua_str = lua_str + "\n"

    data_rows = file.iloc[2:]
    lines = ""
    for index, row in data_rows.iterrows():
        line = "{"
        for cowindex in range(row.values.size):
            if headers.values[cowindex] == "STRING":
                line = line + """{} = "{}",""".format(attrname[cowindex], row.values[cowindex])
            else:
                line = line + """{} = {},""".format(attrname[cowindex], row.values[cowindex])
        line = line + "},\n"
        lines = lines + "\t" + line
    
    lua_str = lua_str + """
---@type {}[]
local {} = {{
{}
}}
return {}
""".format(tabDataName, tabDataManagerName, lines, tabDataManagerName)
    outfile = open(config.LUA_PATH + tabDataName + ".lua", "w", encoding="utf-8")
    outfile.write(lua_str)
    outfile.close()
    

def handle_filepath_tolua(filepath):
    files = os.listdir(filepath)
    for fi in files:
        fi_d = os.path.join(filepath, fi)
        if os.path.isdir(fi_d):
            handle_filepath_tolua(fi_d)
        else:
            handle_excelfile_tolua(fi_d)

def GenLuaTableManagerFile(filepath):
    file = pd.read_excel(filepath)
    filename = os.path.splitext(os.path.basename(filepath))[0]
    tabDataName = "Table" + filename + "Data"
    tabDataManagerName = tabDataName + "Manager"

    lua_str = """
TableManager.__TableList.{} = require "{}"
function TableManager:Get{}ById(Id)
    for i = 1, self:Get{}Count() do
        if self:Get{}ByIndex(i).Id == Id then
            return self:Get{}ByIndex(i)
        end
    end
    return nil
end
function TableManager:Get{}ByIndex(index)
    return self.__TableList.Table{}Data[index]
end
function TableManager:Get{}Count()
    return #self.__TableList.{}
end
""".format(tabDataName, tabDataName,tabDataName,tabDataName,tabDataName,tabDataName,tabDataName,tabDataName,tabDataName,tabDataName,)
    return lua_str

def GenLuaTableManagerDir(filepath):
    lua_str = ""
    files = os.listdir(filepath)
    for fi in files:
        fi_d = os.path.join(filepath, fi)
        if os.path.isdir(fi_d):
            lua_str = lua_str + GenLuaTableManagerDir(fi_d)
        else:
            lua_str = lua_str + GenLuaTableManagerFile(fi_d)
    return lua_str

def GenLuaTableManager(filepath):
    lua_str = "---{}".format(config.WARNING_NOTE) + """

local TableManager = {}
TableManager.__TableList = {}

""" + GenLuaTableManagerDir(filepath) + "\n\nreturn TableManager"
    outfile = open(config.LUA_PATH + "TableManager.lua", "w", encoding="utf-8")
    outfile.write(lua_str)
    outfile.close()
        

def DoExport(filepath):
    handle_filepath_tolua(filepath)
    GenLuaTableManager(filepath)