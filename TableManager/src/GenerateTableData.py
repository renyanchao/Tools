import os
import pandas as pd
import config

import _totxt
import _tocpp
import _tojson
import _tolua

def main():
    
    _tojson.DoExport(config.CONFIG_PATH)
    _totxt.DoExport(config.CONFIG_PATH)
    _tocpp.DoExport(config.CONFIG_PATH)
    _tolua.DoExport(config.CONFIG_PATH)

main()