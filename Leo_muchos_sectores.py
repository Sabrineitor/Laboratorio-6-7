import matplotlib.pyplot as plt
from scipy.io import wavfile as wav
from scipy import signal
import numpy as np
import glob
from subprocess import call
from io import StringIO
from matplotlib import pyplot
import sys

frec_adq = 23000

#tiempo = np.arange(0, len(datos)*1/frec_adq, 1/frec_adq)
#tiempo = np.arange(0, len(datos)*dt_adq, dt_adq)

programa = 'read'               # Programa compilado a partir de readSd.c

ttot = 120 #en segundos
tmed  = 3000.0 * 256 /frec_adq # Duracion de cada medicion: cantidad de sectores medidos por el tamaño del sector
								#tiene que coincidir el numero de sectores en ambos programas!!!

print(tmed)
cantidad = int(ttot/tmed)       # Cantidad de mediciones.
print(cantidad)

Datos = [] #es vector de vectores

for j in range(cantidad):
    # Ejecuta el programa de lectura de 1 medicion (ie 400 sectores). La salida
    # del programa es un file dec.dat con los valores 
    call(["./" + programa, str(int(j))]) #Agregar arg al programa? 
    #sin el str j tengo 3 audios iguales
    # que es el j????
    datos1 = np.loadtxt('dec.dat') #CONSULTAR SI PODEMOS TENER 3 DEC
    #Datos.append(datos1)
    sonido_normalizado = datos1/np.max(np.abs(datos1))
    sonido_normalizado = sonido_normalizado * 32767 #para tener int16 deforma el volumen 
    sonido_normalizado = sonido_normalizado.astype('int16')
    wav.write('audio_' + format(j) + '.wav',int(frec_adq),sonido_normalizado)



sys.exit(0)




datos = []
for i in range(len(Datos)):
	for j in range(len(Datos[i])):
		datos.append(Datos[i][j])

print(len(datos))
#print(datos)

sonido_normalizado = datos/np.max(np.abs(datos))
sonido_normalizado = sonido_normalizado * 32767 #para tener int16 deforma el volumen 
sonido_normalizado = sonido_normalizado.astype('int16')
wav.write('audio.wav',int(frec_adq),sonido_normalizado)


plt.show()
