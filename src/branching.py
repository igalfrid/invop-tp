#!/usr/bin/python

from os import listdir
from os.path import isfile, join

def files(path):
  return [f for f in listdir(path) if isfile(join(path, f))]

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
  path = "./instancias/branching/solved"
  archivos = files(path)
  
  sols = []
  
  for x in archivos:
    nodos, densidad, particiones, i, j, varsel, nodesel = parseFilename(x)
    tt, ta = parseSolution(join(path, x))
    sols.append((x, nodos, densidad, particiones, i, j, varsel, nodesel, tt, ta))
    
  print "archivo,nodos,densidad,particiones,i,j,varsel,nodesel,tt,ta"
  for sol in sorted(sols):
    print ",".join(map(str, sol)) 

def parseFilename(x):
  fields = x.split("-")
  "instancia-$nodos-$densidad-$particiones.$i.$j.$varsel.$nodeSel.col.solved"
  nodos = fields[1]
  densidad = fields[2]
  fields = fields[3].split(".")
  particiones = fields[0]
  fields = x.split(".")
  i = fields[1]
  j = fields[2]
  varsel = fields[3]
  nodesel = fields[4]
  return nodos, densidad, particiones, i, j, varsel, nodesel
if __name__ == "__main__":
  main()
