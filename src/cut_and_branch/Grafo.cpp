#include "Grafo.h"
#include <iostream>

Grafo::Grafo() {
  // No hace nada
}

Grafo::Grafo(int cantidadDeNodos) {
  this->cantidadDeNodos = cantidadDeNodos;
}

void Grafo::setCantidadDeNodos(int cantidad) {
  this->cantidadDeNodos = cantidad;
}

int Grafo::getCantidadDeNodos() {
  return this->cantidadDeNodos;
}

int Grafo::getCantidadDeAristas() {
  return this->aristas.size();
}

int Grafo::getCantidadDeParticiones() {
  return this->particiones.size();
}

void Grafo::agregarArista(int origen, int destino) {
  //Las aristas deben pertenecer al grafo
  if(origen < 1 || origen > cantidadDeNodos) {
    cout << "El nodo origen (" << origen << ") no pertenece al grafo con " << cantidadDeNodos << " nodos." << endl;
    throw;
  }
  if(destino< 1 || destino > cantidadDeNodos) {
    cout << "El nodo destino (" << destino << ") no pertenece al grafo con " << cantidadDeNodos << " nodos." << endl;
    throw;
  }

  // Siempre el origen es menor que el destino
  int origenArista, destinoArista;
  if(origen > destino) {
    // Si el destino es menor que el origen llamo invirtiendo los parametros
    this->agregarArista(destino, origen);
    return;
  }
  // Aca puedo asegurar que el origen es menor que el destino
  if(this->sonAdyacentes(origen, destino)) {
    // Si ya son adyacentes no vuelvo a agregar la arista
    return;
  }
  Arista arista = Arista(origen, destino);
  aristas.push_back(arista);
}

void Grafo::agregarParticion(Particion particion) {
  particiones.push_back(particion);
}

bool Grafo::sonAdyacentes(int p, int q) {
  // El origen es menor que el destino
  if(p > q) {
    //Estan con el orden cambiados, los llamo con el orden correcto
    return this->sonAdyacentes(q, p);
  }

  // Aca puedo asegurar que estan con el orden correcto
  list<Arista>::iterator it;
  for (it=aristas.begin(); it!=aristas.end(); ++it) {
    if(it->getOrigen() == p && it->getDestino() == q) {
      return true;
    }
  }
  return false;
}

list<Arista> Grafo::getAristas() {
  return this->aristas;
}

list<Particion> Grafo::getParticiones() {
  return this->particiones;
}

bool Grafo::esAdyacenteATodos(list<int> nodos, int nodo) {
  list<int>::iterator it;
  for (it=nodos.begin(); it!=nodos.end(); ++it) {
    if(!this->sonAdyacentes(nodo, *it)) {
      return false;
    }
  }
  return true;
}
