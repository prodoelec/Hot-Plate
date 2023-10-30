import time             # Importa módulo de tiempo
import serial           # Importa módulo control puerto serie
from io import open     # Importa módulo ficheros

dato    = ""
linea   = ""
hora    = ""
fecha   = ""
ruta    = "../registros/registro.csv"
archivo = open(ruta, "a")
arduino = serial.Serial()      # Crea el objeto del puerto serie

#**********************************************************************************#
# Configuración puerto serie                                                       #
#**********************************************************************************#

def configuraSerie(port):
    arduino.port = port
    arduino.baudrate = 9600  # Establece la velocidad de comunicación en 9600 baudios
    arduino.bytesize = 8     # Tamañoo del dato a enviar 8 bits
    arduino.parity = 'N'     # Sin paridad
    arduino.stopbits = 1     # Un bit de stop
    arduino.timeout = 1      # Tiempo de espera 1s
    arduino.xonxoff = 0      # Sin control de flujo por software
    arduino.rtscts = 0       # Sin control de flujo por hardware
    arduino.dsrdtr = 0       # Sin control de flujo por hardware
    arduino.open()           # Abre el puerto 
    return

#**********************************************************************************#
# Leer el puerto serie.Devuelve el valor leido en el puerto                        #
#**********************************************************************************#

def lee_serie():
    serie = arduino.readline()  # carga en dato el valor leido del puerto serie
    dato = serie.decode()       # Desde arduino los datos vienen en byte esta
                                # función los convierte a string
    dato = dato[:-2]
    return(dato)                # retorna el dato

#**********************************************************************************#
# Escribir en el puerto serie                                                      #
#**********************************************************************************#

def escribe_serie(dato):
    dato = dato
    arduino.write(dato.encode())
    return

#**********************************************************************************#
# Calcular fecha actual.Devuelve la fecha en formato 2012/12/31                    #
#**********************************************************************************#

def f_actual():
    t = time.time()
    tiempo = time.localtime(t)
    fecha = time.strftime("%d/%m/%Y", tiempo)
    return (fecha)

#**********************************************************************************#
# Calcular hora actual.Devuelve la hora en formato 01:01                           #
#**********************************************************************************#

def h_actual():
    t = time.time()
    tiempo = time.localtime(t)
    hora = time.strftime("%H:%M:%S", tiempo)
    return (hora)

#**********************************************************************************#
# Añade datos a fichero                                                            #
#**********************************************************************************#

def escribe_fichero(linea):
    linea = linea + "\n"
    archivo.write(linea)
    return
