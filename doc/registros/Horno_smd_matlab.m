clear
clc

filename = 'registro.xlsx';
sheet = 1;
samples = 'A2:A732';
temperatura = 'C2:C732';

t = xlsread(filename,sheet,samples)
y = xlsread(filename,sheet,temperatura)

datos = iddata(y,t,1)