import matplotlib.pyplot as plt
from scipy.io import wavfile as wav
import numpy as np
from io import StringIO


dt_adq = 62*10**-6     # Intervalo entre mediciones. Modificar unicamente  (TACCR0)                        
frec_adq = 16000     #Frecuencia de adquicisión en Hz
#frec_adq = 1/dt_adq 

### Agarramos el archivo 'dec.dat' y creamos una string de datos ### 
M = open('dec.dat').read().replace(',','.')
datos = np.loadtxt(StringIO(M))

### Generamos una string de tiempo con la cantidad de datos y la frec_adq ###
tiempo = np.arange(0, len(datos)*1/frec_adq, 1/frec_adq)

### Convertimos los datos a formato wav ###
sonido_normalizado = datos/np.max(np.abs(datos))
sonido_normalizado = sonido_normalizado * 32767 
sonido_normalizado = sonido_normalizado.astype('int16')

### Creamos el archivo '.wav' y se guarda en la carpeta ###
wav.write('ampli.wav',int(frec_adq),sonido_normalizado)

### Plotteamos 'datos' en función de número de dato ###
plt.figure(1)
plt.plot( datos, 'o-')
plt.tick_params(labelsize=13)
plt.legend()
plt.xlabel('cantidad de datos', size = 13)
plt.grid(True)

### Plotteamos 'datos' en función del tiempo ###
plt.figure(2)
plt.plot( tiempo, datos, 'o-')
plt.tick_params(labelsize=13)
plt.legend()
plt.grid(True)

### Muestra los gráficos ###
plt.show()
plt.show()
