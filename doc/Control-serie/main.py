"""
Control Bluetooth de Horno SMD, para depuracion y ajuste de parametros PID
Las funciones que podemos hacer serán las siguientes: 
- (A) Apagar el horno
- (E) Encender horno
- (T) Temperatura de cosigna
- (P) Costante Proporcional (P)
- (I) Contante Integral (I)
- (D) Constante Derivada (D)
- (V) Valores almacenados PID
- (X) Salir
"""
from registro import registro
import os
import time

accion = ""
registro.configuraSerie("COM6")
num_muestras = 10
conta = 0

os.system("cls")

#----------------------------------------------------------------------------------#
# Muestra Menu                                                                     #
#----------------------------------------------------------------------------------#

def muestraMenu():   
   
    print("\nControl Horno SMD")
    print("=================")
    print("""\n 
           (A) Apagar horno
           (E) Encender horno
           (T) Temperatura de consigna
           (P) Proporcional (P)
           (I) Integral (I)
           (D) Derivada (D)
           (V) Valores almacenados PID
           (R) Registro
           (H) Ayuda
           (X) Salir
    """)
    return 

muestraMenu()

while ( accion != "X"):
    
    accion = input("comando> ")
    accion = accion.upper()      

    if accion == "A":
        registro.escribe_serie("A")
        print(registro.lee_serie())
        
    elif accion == "E":
        registro.escribe_serie("E")
        print(registro.lee_serie())
        
    elif accion == "T":
        temperatura = input("\nTemperatura consigna (ºC): ")
        temperatura = "T" + temperatura
        registro.escribe_serie(temperatura)

    elif accion == "P":
        kp = input("\nIntroduce valor kp: ")
        kp = "P" + kp + "\n"
        registro.escribe_serie(kp)
        print(registro.lee_serie())

    elif accion == "I":
        ti = input("\nIntroduce valor ti: ")
        ti = "I" + ti
        registro.escribe_serie(ti)

    elif accion == "D":
        td = input("\nIntroduce valor td: ")
        td = "D" + td
        registro.escribe_serie(td)
    
    elif accion == "V":
        registro.escribe_serie("V")
        print(registro.lee_serie())
    
    elif accion == "R":
               
        num_muestras = int(input("\nIntroduce numero de muestras: "))
        num_muestras -= 1
        cabecera = "Set,Pwr(%),Temperatura"
        registro.escribe_fichero(cabecera)
        
        registro.escribe_serie("R") 
        print(registro.lee_serie())
        
        while( conta <= num_muestras):
            dato = registro.lee_serie()
            print(dato)             
            registro.escribe_fichero(dato) 
            conta += 1
            
        registro.escribe_serie("W") 
        registro.archivo.close()
        conta = 0 
    
    elif accion == "H":
        os.system("cls")
        muestraMenu()      
      
    elif accion == "X":
        print("\nHasta pronto !!")
        registro.archivo.close()
        registro.arduino.close()
        exit
        
    else:
        print("\nIntroduce una opción válida !!!")
        
 


