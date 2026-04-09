import os
import pandas as pd
import config

def handle_excelfile_totxt(filename):
    print("GenerateTableData.py handle_excelfile_totxt", filename)
    file = pd.read_excel(filename)
    filename = os.path.splitext(os.path.basename(filename))[0]
    tabDataName = "Table" + filename + "Data"
    txt_path = config.TXT_PATH + tabDataName + '.txt'
    file = file.iloc[2:]
    file.to_csv(txt_path, sep="\t", index=False, encoding="utf-8") ##output .txt

def handle_filepath_totxt(filepath):
    files = os.listdir(filepath)
    for fi in files:
        fi_d = os.path.join(filepath, fi)
        if os.path.isdir(fi_d):
            handle_filepath_totxt(fi_d)
        else:
            handle_excelfile_totxt(fi_d)

def DoExport(filepath):
    handle_filepath_totxt(filepath)