#include <string>
#include <vector>
#include <list>
#include <set>
#include <algorithm>
#include <random>
#include <iostream>
#include <fstream>

using namespace std;

void generarParticiones(int cantidadDeNodos, int cantidadDeParticiones,
                            vector<set<int>>& particiones) {

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
}

void escribirParticiones(ofstream &archivo, vector<set<int>>& particiones, int cantidadDeParticiones) {
  // Escribo las particiones en el archivo
  for (int i = 0; i < cantidadDeParticiones; i++) {
    archivo << "v " << particiones[i].size();
    for (const auto &n : particiones[i]) {
      archivo << " " << n;
    }

    archivo << endl;
  }
}

int main(int argc, char **argv) {
  if (argc < 4) {
    cerr << "Uso: " << argv[0]
         << " <inputfile> <outputfile> <particiones>"
         << endl;
  }

  ifstream archivoDeEntrada;
  char * nombreDelArchivoDeEntrada = argv[1];
  char * nombreDelArchivoDeSalida = argv[2];
  int cantidadDeParticiones = atoi(argv[3]);
  int cantidadDeNodos, cantidadDeAristas;

  archivoDeEntrada.open(nombreDelArchivoDeEntrada);
  if(archivoDeEntrada.fail()) {
    cout << "No se pudo abrir el archivo: " << nombreDelArchivoDeEntrada << endl;
    exit(1);
  }

  bool encontreLineaDescripcion = false;
  string tipoDeLinea, descarte;
  while(!archivoDeEntrada.eof() && !encontreLineaDescripcion) {
    archivoDeEntrada >> tipoDeLinea;
    if(tipoDeLinea == "p") {
      // Es la linea que describe cantidad de nodos y de aristas
      archivoDeEntrada >> descarte;
      archivoDeEntrada >> cantidadDeNodos;
      archivoDeEntrada >> cantidadDeAristas;
      encontreLineaDescripcion = true;
    }
  }

  // Genero las particiones
  vector<set<int>> particiones(cantidadDeNodos);
  generarParticiones(cantidadDeNodos, cantidadDeParticiones, particiones);

  // Comienzo a escribir el archivo
  ofstream archivoDeSalida(nombreDelArchivoDeSalida);
  archivoDeSalida << "p edge " << cantidadDeNodos << " " << cantidadDeAristas << " " << cantidadDeParticiones << endl;

  // Escribimos las particiones
  escribirParticiones(archivoDeSalida, particiones, cantidadDeParticiones);

  int origen, destino;
  while(!archivoDeEntrada.eof() && cantidadDeAristas > 0) {
    archivoDeEntrada >> tipoDeLinea;
    if(tipoDeLinea == "e") {
      // Es una arista
      archivoDeEntrada >> origen;
      archivoDeEntrada >> destino;

      archivoDeSalida << "e " << origen << " " << destino << endl;
      cantidadDeAristas--;
    }
  }

  // Cierro archivos
  archivoDeEntrada.close();
  archivoDeSalida.close();

  return 0;
}


