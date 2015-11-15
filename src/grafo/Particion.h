#ifndef PARTICION_H
#define PARTICION_H
#include <list>

using namespace std;

class Particion {
    private:
        list<int> nodos;
        int nodo;
    public:
        void agregarNodo(int nodo);
        int getCantidadDeNodos();
        list<int> getNodos();
};

#endif
