import sys
import pandas as pd
#sudo pip install xlrd
#Sudo pip install pandas
#Excel File should be in the same directory

excelfilepath = ('./cs.xlsx')

bit_cols = list(range(1,36))
xls = pd.ExcelFile(excelfilepath)
df = xls.parse('Sheet1', skiprows=0, parse_cols=bit_cols)
x = df.to_string(col_space=0,index=False, header=None)
x = x.replace(" ", "")

ucodefilename = 'ucode'
sys.stdout = open(ucodefilename, 'w')
print(x)

#Verify with .linux file 
