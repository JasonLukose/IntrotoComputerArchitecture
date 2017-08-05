import sys
import pandas as pd
#sudo pip install xlrd
#Sudo pip install pandas
#Excel File should be in the same directory

excelfilepath = ('./control_store_lab3.xls')

bit_cols = list(range(1,24))
xls = pd.ExcelFile(excelfilepath)
df = xls.parse('Sheet1', skiprows=0, parse_cols=bit_cols)
x = df.to_string(col_space=0,index=False, header=None)
x = x.replace(" ", "")

ucodefilename = 'ucode3'
sys.stdout = open(ucodefilename, 'w')
print(x)

#Verify with .linux file 
