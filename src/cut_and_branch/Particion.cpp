#include "Particion.h"

void Particion::agregarNodo(int nodo) {
  this->nodos.push_back(nodo);
}

int Particion::getCantidadDeNodos() {
  return this->nodos.size();
}

list<int> Particion::getNodos() {
  return this->nodos;
}
