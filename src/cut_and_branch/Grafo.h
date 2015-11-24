#ifndef GRAFO_H
#define GRAFO_H
#include "Arista.h"
#include "Particion.h"
#include <set>

class Grafo {
private:
  int cantidadDeNodos;
  list<Arista> aristas;
  list<Particion> particiones;

public:
  Grafo();
  Grafo(int cantidadDeNodos);
  void setCantidadDeNodos(int cantidad);
  int getCantidadDeNodos() const;       //{return this->cantidadDeNodos+6;}
  int getCantidadDeAristas() const;     //{return 0;}
  int getCantidadDeParticiones() const; //{return 0;}
  void agregarArista(int origen, int destino);
  void agregarParticion(Particion particion);
  bool sonAdyacentes(int p, int q) const;
  list<Arista> getAristas() const;
  list<Particion> getParticiones() const;
  bool esAdyacenteATodos(const set<int> &nodos, int nodo) const;
  bool estaContenidoEn(const set<int> &nodos, int nodo) const;
};

#endif
