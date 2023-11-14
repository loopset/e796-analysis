#!/usr/bin/env python
# coding: utf-8

### SRIM data transformer and density calculator


import pandas as pd
import numpy as np

'''
Column name according to:
 - Capital letters for value
 - Other for units (Stopping powerd dont have units)
'''

#WARNING: TAKE HEED IN skiprows and skipfooter, because they depend on number of gases set in SRIM!!! (varies between 24-25-26-27)
def read_convert_write(indirectory, ficheiro, outdirectory):

    df = pd.read_csv(str(indirectory + ficheiro + '.txt'), skiprows = 24, skipfooter = 13,
                     delim_whitespace = True, header = None, decimal = ',',
                     engine='python')
    
    #df = df.drop([2,3,8,9], axis = 1)
    
    df.columns = ['E', 'e', 'Se', 'Sn', 'R', 'r', 'LS', 'ls', 'LatS', 'lats']
    
    ## workaround because srim outputs 1,02E+5 values, not converted by default to float by pandas
    for col in ['Se', 'Sn']:
    	if df[col].dtype != np.float64:
    	    df[col] = df[col].str.replace(',', '.').astype(np.float64)
    
    
    enerxia = {'eV' : 1e-6, 'keV': 1e-3, 'GeV': 1e3,}

    distancia = {'A': 1e-7, 'um': 1e-3, 'm':1e3, 'km':1e6,}

    for key, value in enerxia.items():
        df.E = df.E.mask(df.e == key, lambda x: x * value)
        df.e = df.e.mask(df.e == key, 'MeV')
    
    for key, value in distancia.items():
        df.R = df.R.mask(df.r == key, lambda x: x * value)
        df.r = df.r.mask(df.r == key, 'mm')
        
        df.LS = df.LS.mask(df.ls == key, lambda x: x * value)
        df.ls = df.ls.mask(df.ls == key, 'mm')
        
        df.LatS = df.LatS.mask(df.lats == key, lambda x: x * value)
        df.lats = df.lats.mask(df.lats == key, 'mm')

    ##write all columns to file
    df[['E', 'Se', 'Sn', 'R', 'LS', 'LatS']].to_csv(str(outdirectory + ficheiro + ".dat"), sep=' ', index=False, header=False, float_format='%.6f')
    return df

## and now iterate over all of our files
indirectory = './raw/'
outdirectory = './transformed/'
pressures = [140]
particle = 'protons'
#files = [particle + '_in_' + str(pressure) + 'mb_butane' for pressure in pressures]
files = ["protons_deuterium_900mb", "11Li_deuterium_900mb"]
#files = ['alphas_in_50mb_butane']##['protons_in_30mb_butane', 'protons_in_100mb_butane', 'protons_in_500mb_butane']
dfs = []
for file in files:
    ##run function
    dfs.append(read_convert_write(indirectory, file, outdirectory))
