import os
import pandas as pd
import config

def GenCppType(type_str):
    if type_str == "INT":
        return "int32_t"
    if type_str == "STRING":
        return "std::string"
    if type_str == "BOOL":
        return "bool"
    if type_str == "FLOAT":
        return "float"
def GenCppReadlineType(type_str, source_str):
    if type_str == "INT":
        return "std::stoi({})".format(source_str)
    if type_str == "STRING":
        return "std::string({})".format(source_str)
    if type_str == "BOOL":
        return "std::stoi({}) == 1".format(source_str)
    if type_str == "FLOAT":
        return "std::stof({})".format(source_str)
    



def handle_excelfile_tocpp(filename):
    print("GenerateTableData.py handle_excelfile_tojson", filename)
    file = pd.read_excel(filename)
    filename = os.path.splitext(os.path.basename(filename))[0]
    tabDataName = "Table" + filename + "Data"
    headers = file.iloc[0]
    attrname = file.columns.tolist()

    field_define_str = ""
    for i in range(headers.values.size):
        field_define_str = field_define_str + "\t\t"+ GenCppType(headers.values[i]) + "\t" + attrname[i] + ";\n"
    field_read_str = ""
    for i in range(headers.values.size):
        field_read_str = field_read_str + "\t\t\t"+attrname[i] + " = " + GenCppReadlineType(headers.values[i], "fields[{}]".format(i)) + ";\n"

    output_str = """
//{}
#include"TableManager.h"

struct {}
{{
{}
        
    void ReadLine(const std::vector<std::string>& fields)
    {{
{}
    }}
    const static std::string GetFileName()
    {{
        return "{}";
    }}
}};
""".format(config.WARNING_NOTE, tabDataName, field_define_str, field_read_str, filename + ".txt", tabDataName, filename)
    
    outfile = open(config.CPP_PATH + "Table{}Data.h".format(filename), "w", encoding="utf-8")
    outfile.write(output_str)
    outfile.close()



def handle_filepath_tocpp(filepath):
    files = os.listdir(filepath)
    for fi in files:
        fi_d = os.path.join(filepath, fi)
        if os.path.isdir(fi_d):
            handle_filepath_tocpp(fi_d)
        else:
            handle_excelfile_tocpp(fi_d)

def generateCppTableManager_Include_str(filepath):
    include_table_str = ""
    files = os.listdir(filepath)
    for fi in files:
        fi_d = os.path.join(filepath, fi)
        if os.path.isdir(fi_d):
            include_table_str = include_table_str + generateCppTableManager_Include_str(fi_d)
        else:
            filename = os.path.splitext(os.path.basename(fi_d))[0]
            tabDataName = "Table" + filename + "Data"
            include_table_str = include_table_str + """#include "{}.h" \n""".format(tabDataName)
    return include_table_str

def generateCppTableManager_Read_str(filepath):
    read_table_str = ""
    files = os.listdir(filepath)
    for fi in files:
        fi_d = os.path.join(filepath, fi)
        if os.path.isdir(fi_d):
            read_table_str = read_table_str + generateCppTableManager_Read_str(fi_d)
        else:
            filename = os.path.splitext(os.path.basename(fi_d))[0]
            tabDataName = "Table" + filename + "Data"
            read_table_str = read_table_str + "\tTableManager::ReadTable<{}>();\n".format(tabDataName)
    return read_table_str


def generateCppTableManager(filepath):
    include_str = generateCppTableManager_Include_str(filepath)
    read_str = generateCppTableManager_Read_str(filepath)
    outfile = open(config.CPP_PATH + "TableManager.cpp", "w", encoding="utf-8")
    str = """
//{}
#include "TableManager.h"
{}
void TableManager::ReadAllTable()
{{
{}}}

namespace TableManager
{{
    std::unordered_map<std::type_index, std::shared_ptr<void>> g_TableManager;  // 定义
}}
""".format(config.WARNING_NOTE, include_str, read_str)
    outfile.write(str)
    outfile.close()


def DoExport(filepath):
    handle_filepath_tocpp(filepath)
    generateCppTableManager(filepath)