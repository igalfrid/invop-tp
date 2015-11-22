#include <ilcplex/ilocplex.h>
#include <ilcplex/cplex.h>
ILOSTLBEGIN
#include <string>
#include <vector>
#include <list>

using namespace std;

void generarParticiones(int cantidadDeNodos, int cantidadDeParticiones, list<int> * particiones) {
  // Recorro los nodos y coloco a cada nodo en la particion que le corresponde
  for(int i=1; i<=cantidadDeNodos; i++) {
    int particion = rand() % cantidadDeParticiones;
    particiones[particion].push_back(i);
  }
}

void escribirParticiones(ofstream &archivo, list<int> * particiones, int cantidadDeParticiones) {
  for (int i=0; i<cantidadDeParticiones; i++) {
    archivo << "v " << particiones[i].size();
    list<int>::iterator it;
    for (it=particiones[i].begin(); it!=particiones[i].end(); ++it) {
      archivo << " " << *it;
    }
    archivo << endl;
  }
}

int main(int argc, char **argv) {
  ifstream archivoDeEntrada;
  char * nombreDelArchivoDeEntrada = argv[1];
  char * nombreDelArchivoDeSalida = argv[2];
  int cantidadDeParticiones = atoi(argv[3]);
  int cantidadDeNodos, cantidadDeAristas;

  cout << "Nombre del archivo de entrada: " << nombreDelArchivoDeEntrada << endl;
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

  // Ya tenemos la cantidad de nodos
  // Generamos las particiones
  list<int> particiones[cantidadDeParticiones];
  generarParticiones(cantidadDeNodos, cantidadDeParticiones, particiones);

  cout << "Genere las particiones" << endl;

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


