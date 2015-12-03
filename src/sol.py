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
              sols.append((archivo, nodos, densidad, particiones, i, j, algs, tt, ta))

  print "archivo,nodos,densidad,particiones,i,j,algs,tt,ta"
  for sol in sorted(sols):
    print ",".join(map(str, sol)) 
if __name__ == "__main__":
  main()
