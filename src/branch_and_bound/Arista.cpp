#include "Arista.h"

Arista::Arista(int origen, int destino) {
  this->origen = origen;
  this->destino = destino;
}

int Arista::getOrigen() { return this->origen; }

int Arista::getDestino() { return this->destino; }
