from re import X
import pandas as pd
import numpy as np
from matplotlib import pyplot as plt
from google.colab import drive
import seaborn as sns
from scipy import stats
import sqlite3

# Read sqlite query results into a pandas DataFrame
#con = sqlite3.connect("data/portal_mammals.sqlite")
#df = pd.read_sql_query("SELECT * from surveys", con)

# Verify that result of SQL query is stored in the dataframe
#print(df.head())

#con.close()
#import pingouin as pg
#data=pd.read_csv('angie.csv',header=0)
#dat = sqlite3.connect('data.db')
#query = dat.execute("SELECT * From <TABLENAME>")
#cols = [column[0] for column in query.description]
#results= pd.DataFrame.from_records(data = query.fetchall(), columns = cols)

drive.mount("/content/drive/")
data=pd.read_csv('/content/drive/MyDrive/Colab Notebooks/Angie.csv',header=0)

resolucion=1
Tmin=100
#Tref=121.11111111
Tref=121
Z=10
trefseg=60

dataF=pd.DataFrame(data)
for i in range (0,len(dataF)):
    if  dataF.loc[i,'Temperatura']>=Tmin:
        dataF.loc[i,'letalidad']=pow(10, ( (data.loc[i,'Temperatura'])-Tref)/Z)
    else:
         data.loc[i,'letalidad']=0
F0 =dataF['letalidad'].sum()
print("F0=",F0)
std=data['Temperatura'].std()
print("std=",std)
max=data['Temperatura'].max()
print("max=",max)
min=data['Temperatura'].min()
print("min=",min)
mean=data['Temperatura'].mean()
print("mean=",mean)
datatemp=dataF[dataF['Temperatura'].between(108,129)]
print(datatemp)
desviacion_estandar=datatemp.Temperatura.std()
des=desviacion_estandar.astype(float)
print("Desviacion_estandar=",des)
promedio = datatemp.Temperatura.mean().astype(float)
print("Promedio=",promedio)
error= Tref-promedio 
print("Error=",error)
incertidumbre_UA=des/pow(len(datatemp),0.5)
print("Incertidumbre_UA=",incertidumbre_UA)
if promedio*1.75/100> 0.04:
    incertidumbre_UB1=(promedio*1.75/100)/pow(12,0.5)
else :
    incertidumbre_UB1=0.04 /pow(12,)
print("Incertidumbre_UB1=",incertidumbre_UB1)    
incertidumbre_UB2=resolucion/pow(12,0.5)
print("Incertidumbre_UB2=",incertidumbre_UB2)
incertidumbre_UC=pow( (pow(incertidumbre_UA,2))+(pow(incertidumbre_UB1,2))+(pow(incertidumbre_UB2,2)),0.5)
print("Incertidumbre_UC=",incertidumbre_UC)
incertidumbre_UEF=pow(incertidumbre_UC,4)/( (pow(incertidumbre_UA,4)/(len(datatemp)-1))+(pow(incertidumbre_UB1,4)/200)+(pow(incertidumbre_UB2,4)/200)  )
print("Incertidumbre_UEF=",incertidumbre_UEF)
alfa = 0.05
free_deg = incertidumbre_UEF
if  (stats.t.ppf(1-alfa/2,free_deg) ) >= 2 :
    factork=2
else:
    factork=(stats.t.ppf(1-alfa/2,free_deg) )
print("FACTOR K=",factork)
incertidumbre_UEXP=factork*incertidumbre_UC
print("Incertidumbre EXP=",incertidumbre_UEXP)
x = [x for x in range(len(dataF))]
ys = dataF.loc[x,'Temperatura']
plt.plot(x, ys, '-')
plt.fill_between(x, ys, 0, where=(ys > 121), facecolor='r', alpha=0.6)

plt.title("Sample Visualization")
plt.show()
