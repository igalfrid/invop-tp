#include "Arista.h"

Arista::Arista(int origen, int destino) {
  this->origen = origen;
  this->destino = destino;
}

int Arista::getOrigen() const { return origen; }

int Arista::getDestino() const { return destino; }
