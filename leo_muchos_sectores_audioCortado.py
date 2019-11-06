"""
Este programa lee la sd y genera el wav entrecortado (por si el total ocupa mucho espacio)
Se elije ttot tiempo total a leer
tdiv cantidad de audios finales
min_recorte tiempo que quiero cada audio 
en tmed la cantidad de sectores tiene que coincidir con el que esta en ReadSd.c
"""
import matplotlib.pyplot as plt
from scipy.io import wavfile as wav
import numpy as np
import glob
from subprocess import call
from io import StringIO

frec_adq = 23000 

programa = 'read'               # Programa compilado a partir de readSd.c

ttot = 120 #en segundos
tmed  = 3000.0 * 256 /frec_adq # Duracion de cada medicion: cantidad de sectores medidos por el tamaño del sector
								#tiene que coincidir el numero de sectores en ambos programas!!!

#print(tmed) con 3000 sect es 34 seg aprox

cantidad = int(ttot/tmed)       # Cantidad de iteraciones
print('cant de iteraciones ' + str(cantidad))

Datos = [] #es vector de vectores

for j in range(cantidad):
    # Ejecuta el programa de lectura de 1 medicion (ie 400 sectores). La salida
    # del programa es un file dec.dat con los valores de los dos canales
    call(["./" + programa, str(int(j))]) #Agregar arg al programa?
    dat_file = glob.glob('dec.dat')[0]
    datos1 = np.loadtxt('dec.dat')
    Datos.append(datos1)
    

datos = []
for i in range(len(Datos)):
    for j in range(len(Datos[i])):
        datos.append(Datos[i][j])

print('cant de datos leidos ' + str(len(datos)))

#separo el audio 
tdiv = 1 #cant de audios 

min_recorte = 3 #tiempo que quiero el audio 
cant_sectores = int(min_recorte * 60 *3000/33.3)

step = cant_sectores *256 #asi es cant de sectores por la cantidad de datos

for i in range(tdiv):

	datos_audio = []
	for j in range(len(datos)):
		if j <= (i+1)*step and j > (i *step):
			datos_audio.append(datos[j])
	print(len(datos_audio))

	sonido_normalizado = datos_audio/np.max(np.abs(datos_audio))
	sonido_normalizado = sonido_normalizado * 32767 #para tener int16 deforma el volumen 
	sonido_normalizado = sonido_normalizado.astype('int16')
	wav.write('audio_' + format(i) + '.wav',int(frec_adq),sonido_normalizado)


###Audio Completo
"""
sonido_normalizado = datos/np.max(np.abs(datos))
sonido_normalizado = sonido_normalizado * 32767 #para tener int16 deforma el volumen 
sonido_normalizado = sonido_normalizado.astype('int16')
wav.write('audio.wav',int(frec_adq),sonido_normalizado)
"""



##### Paso datos a csv para analisis 
#np.savetxt('todos_los_datos.csv', datos, delimiter = '\t')



##### Grafico
"""
tiempo = np.arange(0, len(datos)*1/frec_adq, 1/frec_adq)

plt.plot( tiempo,datos, 'o-')
plt.tick_params(labelsize=13)
plt.legend()
plt.xlabel('Tiempo [s]', size = 13)
#ax1.set_ylabel('Intensidad [s.u.]')
plt.ylabel('Amplitud', size = 13)
plt.grid(True)
plt.show()
"""
