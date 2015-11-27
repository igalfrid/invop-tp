#!/usr/bin/python
import sys
import json
from random import randint

def parseInstance(filepath):
  nodos = 0
  edges = []
  particiones = []
  with open(filepath, 'r') as f:
    for line in f:
      if line.startswith('p'):
        s = line.split(' ')
        nodos = int(s[2])
      if line.startswith('v'):
        s = line.split(' ')
        particiones.append(map(int, s[2:]))
      if line.startswith('e'):
        s = line.split(' ')
        v, w = int(s[1]), int(s[2])
        edges.append((v, w))
  return nodos, edges, particiones

def parseSolution(filepath):
  nodosPintados = {}
  coloresUsados = {}
  with open(filepath, 'r') as f:
    # Si la linea empieza con x es porque es un nodo
    # Si empieza con w es porque es un color
    for line in f:
      if line.startswith('x'):
        # xj_k = 1
        s = line.split('_')
        nodo = int(s[0][1:])
        
        s = s[1].split('=')
        color = int(s[0][0:])
        nodosPintados[nodo] = color
        if color not in coloresUsados:
          coloresUsados[color] = []
        coloresUsados[color].append(nodo)
  return nodosPintados, coloresUsados

def main(instance, solution):
  inst = parseInstance(instance)
  coloreo = parseSolution(solution)
  
  print esValido(inst, coloreo)

def esValido(instance, coloreo):
  nodos, edges, particiones = instance
  nodosPintados, coloresUsados = coloreo
  
  # Es un coloreo
  for (v, w) in edges:
    if v not in nodosPintados: continue
    if w not in nodosPintados: continue
    if nodosPintados[v] == nodosPintados[w]:
      return False
  
  # Solo un nodo de cada particion esta colorado.
  for p in particiones:
    if len([n for n in p if n in nodosPintados]) != 1:
      return False
  return True

if __name__ == "__main__":
  if len(sys.argv) < 3:
    print "Usage: %s <instancia> <solucion>" % sys.argv[0]
  main(sys.argv[1], sys.argv[2])
