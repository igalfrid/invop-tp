#ifndef ARISTA_H
#define ARISTA_H

using namespace std;
class Arista {
private:
  int origen;
  int destino;

public:
  Arista(int origen, int destino);
  int getOrigen() const;
  int getDestino() const;
};

#endif
