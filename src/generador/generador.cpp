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
  list<pair<int, int>> aristas;
  for (int i = 1; i <= cantidadDeNodos; i++) {
    for (int j = i + 1; j <= cantidadDeNodos; j++) {
      double random = ((double)rand() / (RAND_MAX));
      if (random <= densidadDeAristas) {
        pair<int, int> arista = make_pair(i, j);
        aristas.push_back(arista);
      }
    }
  }
  return aristas;
}

void generarParticiones(ofstream &archivo, int cantidadDeNodos,
                        int cantidadDeParticiones) {
  // Creo las n particiones
  vector<set<int>> particiones(cantidadDeNodos);

  vector<int> nodosDisponibles;
  // Agregamos todos los nodos.
  for (int i = 1; i <= cantidadDeNodos; i++) {
    nodosDisponibles.push_back(i);
  }
  random_shuffle(nodosDisponibles.begin(), nodosDisponibles.end());

  random_device rd; // only used once to initialise (seed) engine
  mt19937 rng(
      rd()); // random-number engine used (Mersenne-Twister in this case)
  uniform_int_distribution<int> uni(0, cantidadDeParticiones - 1);

  for (int i = 0; i < cantidadDeParticiones; i++) {
    particiones[i].insert(nodosDisponibles[i]);
  }

  for (int i = cantidadDeParticiones; i < cantidadDeNodos; i++) {
    auto random_integer = uni(rng);
    particiones[random_integer].insert(nodosDisponibles[i]);
  }

  // Escribo las particiones en el archivo
  for (int i = 0; i < cantidadDeParticiones; i++) {
    archivo << "v " << particiones[i].size();
    for (const auto &n : particiones[i]) {
      archivo << " " << n;
    }

    archivo << endl;
  }
}

void escribirAristas(ofstream &archivo, list<pair<int, int>> aristas) {
  list<pair<int, int>>::iterator it;
  for (it = aristas.begin(); it != aristas.end(); ++it) {
    archivo << "e " << it->first << " " << it->second << endl;
  }
}

int main(int argc, char **argv) {
  if (argc < 5) {
    cerr << "Uso: " << argv[0]
         << " <nodos> <densidad (entre 0 y 1)> <particiones> <outputfile>"
         << endl;
  }
  int cantidadDeNodos = atoi(argv[1]);
  double densidadDeAristas = atof(argv[2]);
  int cantidadDeParticiones = atoi(argv[3]);
  char *nombreDeArchivo = argv[4];

  // Genero las aristas
  list<pair<int, int>> aristas =
      generarAristas(cantidadDeNodos, densidadDeAristas);

  // Comienzo a escribir el archivo
  ofstream archivo(nombreDeArchivo);
  archivo << "c Instancia generada aleatoriamente" << endl;
  archivo << "p edge " << cantidadDeNodos << " " << aristas.size() << " "
          << cantidadDeParticiones << endl;

  // Genero las particiones
  generarParticiones(archivo, cantidadDeNodos, cantidadDeParticiones);

  // Escribo aristas
  escribirAristas(archivo, aristas);

  // Cierro el archivo
  archivo.close();

  return 0;
}
