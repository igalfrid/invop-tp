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
  path = "./instancias/GSIA/solved"
  archivos = files(path)
  
  sols = []
  
  for x in archivos:
    inst, partitions, i, algs = parseFilename(x)
    tt, ta = parseSolution(join(path, x))
    sols.append((x, inst, partitions, i, algs, tt, ta))
    
  print "archivo,instancia,particiones,i,algs,tt,ta"
  for sol in sorted(sols):
    print ",".join(map(str, sol)) 

def parseFilename(x):
  fields = x.split("-")
  "miciel4"
  inst = fields[0]
  fields = fields[1].split(".")
  "10.1.col.19"
  partitions, i, algs = fields[0], fields[1], fields[3]
  return inst, partitions, i, algs
if __name__ == "__main__":
  main()
