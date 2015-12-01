#include <string>
#include <vector>
#include <list>
#include <set>
#include <algorithm>
#include <random>
#include <iostream>
#include <fstream>

using namespace std;

list<pair<int, int>> generarAristas(int cantidadDeNodos,
                                    double densidadDeAristas) {
  random_device rd; // only used once to initialise (seed) engine
  mt19937 rng(rd());
  uniform_real_distribution<> uni(0, 1);

  list<pair<int, int>> aristas;
  for (int i = 1; i <= cantidadDeNodos; i++) {
    for (int j = i + 1; j <= cantidadDeNodos; j++) {
      auto random = uni(rng);
      if (random <= densidadDeAristas) {
        pair<int, int> arista = make_pair(i, j);
        aristas.push_back(arista);
      }
    }
  }
  return aristas;
}

void escribirAristas(ofstream &archivo, list<pair<int, int>> aristas) {
  list<pair<int, int>>::iterator it;
  for (it = aristas.begin(); it != aristas.end(); ++it) {
    archivo << "e " << it->first << " " << it->second << endl;
  }
}

int main(int argc, char **argv) {
  if (argc < 4) {
    cerr << "Uso: " << argv[0]
         << " <nodos> <densidad (entre 0 y 1)> <outputfile>"
         << endl;
  }
  int cantidadDeNodos = atoi(argv[1]);
  double densidadDeAristas = atof(argv[2]);
  char *nombreDeArchivo = argv[3];

  // Genero las aristas
  list<pair<int, int>> aristas =
      generarAristas(cantidadDeNodos, densidadDeAristas);

  // Comienzo a escribir el archivo
  ofstream archivo(nombreDeArchivo);
  archivo << "c Instancia generada aleatoriamente" << endl;
  archivo << "p edge " << cantidadDeNodos << " " << aristas.size() << endl;

  // Escribo aristas
  escribirAristas(archivo, aristas);

  // Cierro el archivo
  archivo.close();

  return 0;
}
