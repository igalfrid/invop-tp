#include "Grafo.h"
#include <iostream>
#include <vector>

Grafo::Grafo() {
  // No hace nada
}

Grafo::Grafo(int cantidadDeNodos) { setCantidadDeNodos(cantidadDeNodos); }

void Grafo::setCantidadDeNodos(int cantidad) {
  this->cantidadDeNodos = cantidad;

  adyacentes.clear();
  // usamos los indices desde 1 hasta cantidad inclusive
  for (int i = 0; i < cantidad + 1; i++) {
    vector<bool> row(cantidad + 1, false);
    adyacentes.push_back(row);
  }
}

int Grafo::getCantidadDeNodos() const { return cantidadDeNodos; }

int Grafo::getCantidadDeAristas() const { return aristas.size(); }

int Grafo::getCantidadDeParticiones() const { return particiones.size(); }

void Grafo::agregarArista(int origen, int destino) {
  // Las aristas deben pertenecer al grafo
  if (origen < 1 || origen > cantidadDeNodos) {
    cout << "El nodo origen (" << origen << ") no pertenece al grafo con "
         << cantidadDeNodos << " nodos." << endl;
    throw;
  }
  if (destino < 1 || destino > cantidadDeNodos) {
    cout << "El nodo destino (" << destino << ") no pertenece al grafo con "
         << cantidadDeNodos << " nodos." << endl;
    throw;
  }

  // Siempre el origen es menor que el destino
  if (origen > destino) {
    // Si el destino es menor que el origen llamo invirtiendo los parametros
    agregarArista(destino, origen);
    return;
  }

  // Aca puedo asegurar que el origen es menor que el destino
  if (sonAdyacentes(origen, destino)) {
    // Si ya son adyacentes no vuelvo a agregar la arista
    return;
  }
  Arista arista = Arista(origen, destino);
  aristas.push_back(arista);

  adyacentes[origen][destino] = true;
  adyacentes[destino][origen] = true;
}

void Grafo::agregarParticion(Particion particion) {
  particiones.push_back(particion);
}

bool Grafo::sonAdyacentes(int p, int q) const { return adyacentes[p][q]; }

list<Arista> Grafo::getAristas() const { return aristas; }

list<Particion> Grafo::getParticiones() const { return particiones; }

bool Grafo::esAdyacenteATodos(const set<int> &nodos, int nodo) const {
  for (const auto &n : nodos) {
    if (not sonAdyacentes(nodo, n)) {
      return false;
    }
  }

  return true;
}

bool Grafo::estaContenidoEn(const set<int> &nodos, int nodo) const {
  for (const auto &n : nodos) {
    if (n == nodo) {
      return true;
    }
  }

  return false;
}
