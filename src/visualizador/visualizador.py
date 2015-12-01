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

def dibujarGrafo(inst, coloreo):
  nodos, aristas, particiones = inst
  nodosPintados, coloresUsados = coloreo
  lsnodos = []
  lsaristas = []
  
  
  color = lambda n: nodosPintados[n] if n in nodosPintados else -1
  cs = {"red": "#FF0000", "white":	"#FFFFFF", "cyan":	"#00FFFF", "silver": "#C0C0C0", "blue":	"#0000FF", 	"gray" :"#808080", "darkblue":	"#0000A0",	"black": "#000000", "lightblue":	"#ADD8E6",	"orange":	"#FFA500", "purple":	"#800080",	"brown": "#A52A2A", "yellow":	"#FFFF00",	"maroon": "#800000", "lime":	"#00FF00",	"green": "#008000", "magenta":	"#FF00FF",	"olive": "#808000"}
  css = [cs[x] for x in "lightblue red blue yellow orange green cyan brown darkblue white purple lime magenta".split(" ")]
  
  colores = {}
  i = 0
  for c in coloresUsados:
    colores[c] = css[i]
    i += 1
  
  i = 0
  shapes = "ellipse circle database box test image circularImage diamond dot star triangle triangleDown square icon".split(" ")
  for p in particiones:
    i += 1
    posPintados = 0
    posNodos = 1000
    for n in p:
      pos = posNodos
      posNodos += 100
      c = "lightblue"
      if n in nodosPintados:
        pos = posPintados + randint(0, 500)
        c = colores[color(n)]
      lsnodos.append({"id": n, "label": str(n), "color": c, "level": i, "physics": False, "font": {"color": "white", "background": "black"}, "y": pos})
  
  for (v, w) in aristas:
    color = "gray"
    if v in nodosPintados and w in nodosPintados:
      color = "orange"
    lsaristas.append({"from": v, "to": w, "value": 1, "color": color})
  
  imprimirvis({"nodes": lsnodos, "links": lsaristas})

def imprimirvis(grafo):
  print "var nodes = ", json.dumps(grafo["nodes"]), ";"
  print "var edges = ", json.dumps(grafo["links"]), ";"
  

def main(instance, solution):
  inst = parseInstance(instance)
  coloreo = parseSolution(solution)

  dibujarGrafo(inst, coloreo)
if __name__ == "__main__":
  if len(sys.argv) < 3:
    print "Usage: %s <instancia> <solucion>" % sys.argv[0]
  main(sys.argv[1], sys.argv[2])
