#!/usr/bin/python
import sys
from collections import namedtuple

def parseSolution(filepath):
  tiempototal = -1;
  tiempoarbol = -1;
  try:
    with open(filepath, 'r') as f:
      # Si la linea empieza con x es porque es un nodo
      # Si empieza con w es porque es un color
      for line in f:
        if line.startswith('Tiempo de corrida total'):
          tiempototal = float(line.split(' ')[-1])
        if line.startswith('Tiempo de corrida recorrer el arbol'):
          tiempoarbol = float(line.split(' ')[-1])
  except:
    pass
  return tiempototal, tiempoarbol;

def main():
  soluciones = {}
  sols = []
  for nodos in [30, 40, 50]:
    for densidad in [10, 25, 50, 75, 90]:
      for particiones in [5, 10, 15, 20]:
        for i in xrange(1, 6):
          for j in xrange(1, 6):
            for algs in [0, 3, 8, 11, 19]:
              archivo = "instancias/solved/instancia-%d-%d-%d.%d.%d.col.%d" % (nodos, densidad, particiones, i, j, algs)
              tt, ta = parseSolution(archivo)
              if (tt == -1):
                continue
              soluciones[archivo] = (tt, ta)
              sols.append(((nodos, densidad, particiones, i, j, algs), (tt, ta) )) 
  Res = namedtuple("Res", ["nodos", "densidad", "particiones", "algs", "i", "j", "tt", "ta"])
  ds = []
  for (key, val) in sols:
    k = Res(nodos=key[0], densidad=key[1], particiones=key[2], 
            i = key[3], j = key[4], algs = key[5],
            tt=val[0], ta = val[1])
    ds.append(k)             
  # en ds podemos hacer ds[0].nodos
  promediar(ds)

def promediar(ds):
  Clave = namedtuple("Clave", ["nodos", "densidad", "particiones", "algs", "i"])
  dic = {}
  for d in ds:
    c = Clave(nodos=d.nodos, densidad=d.densidad, particiones=d.particiones, algs=d.algs, i=d.i)
    if c not in dic:
      dic[c] = []
    dic[c].append((d.tt, d.ta))
  
  for k in dic:
    dic[k] = (avg([t[0] for t in dic[k]]), avg([t[1] for t in dic[k]]))

  # Ahora calculamos los promedios de estos promedios (agrupamos por i)
  Clave = namedtuple("Clave", ["nodos", "densidad", "particiones", "algs"])
  dic2 = {}
  for k in dic:
    c = Clave(nodos=k.nodos, densidad=k.densidad, particiones=k.particiones, algs=k.algs)
    if c not in dic2:
      dic2[c] = []
    dic2[c].append(dic[k])

  for k in dic2:
    dic2[k] = (avg([t[0] for t in dic2[k]]), avg([t[1] for t in dic2[k]]))
  
  for k in sorted(dic2.keys()):
    print k, dic2[k]
  
def avg(ls):
  return sum(ls)/float(len(ls))
  
if __name__ == "__main__":
  main()
