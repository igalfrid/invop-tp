#ifndef GRAFO_H
#define GRAFO_H
#include "Arista.h"
#include "Particion.h"
#include <list>

class Grafo {
    private:
        int cantidadDeNodos;
        list<Arista> aristas;
        list<Particion> particiones;

    public:
        Grafo();
        Grafo(int cantidadDeNodos);
        void setCantidadDeNodos(int cantidad);
        int getCantidadDeNodos();//{return this->cantidadDeNodos+6;}
        int getCantidadDeAristas();//{return 0;}
        int getCantidadDeParticiones();//{return 0;}
        void agregarArista(int origen, int destino);
        void agregarParticion(Particion particion);
        bool sonAdyacentes(int p, int q);
        list<Arista> getAristas();
        list<Particion> getParticiones();
        bool esAdyacenteATodos(list<int> nodos, int nodo);
};


#endif
