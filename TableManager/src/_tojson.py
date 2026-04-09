import os
import pandas as pd
import config


def handle_excelfile_tojson(filename):
    print("GenerateTableData.py handle_excelfile_tojson", filename)
    file = pd.read_excel(filename)
    filename = os.path.splitext(os.path.basename(filename))[0]
    tabDataName = "Table" + filename + "Data"
    json_path = config.JSON_PATH + tabDataName + '.json'
    file = file.iloc[2:]
    file.to_json(json_path, orient="records", force_ascii=False, indent=2)

def handle_filepath_tojson(filepath):
    files = os.listdir(filepath)
    for fi in files:
        fi_d = os.path.join(filepath,fi)
        if os.path.isdir(fi_d):
            handle_filepath_tojson(fi_d)
        else:
            handle_excelfile_tojson(fi_d)

def DoExport(filepath):
    handle_filepath_tojson(filepath)