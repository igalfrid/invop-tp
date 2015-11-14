#include <ilcplex/ilocplex.h>
#include <ilcplex/cplex.h>
ILOSTLBEGIN
#include <string>
#include <vector>

int generarAristas(ofstream &archivo, int cantidadDeNodos, double densidadDeAristas) {
  int cantidadDeAristas = 0;
  for(int i = 1; i <= cantidadDeNodos; i++) {
    for(int j = i + 1; j <= cantidadDeNodos; j++) {
      double random = ((double) rand() / (RAND_MAX));
      if(random <= densidadDeAristas) {
        archivo << "e " << i << " " << j << endl;
        cantidadDeAristas++;
      }
    }
  }
  return cantidadDeAristas;
}

int generarParticion(ofstream &archivo, int cantidadDeNodosPorParticion, int nodoActual) {
  archivo << "v " << cantidadDeNodosPorParticion;
  for(int i = 0; i < cantidadDeNodosPorParticion; i++) {
    archivo << " " << nodoActual;
    nodoActual++;
  }
  archivo << endl;
  return nodoActual;
}

void generarParticiones(ofstream &archivo, int cantidadDeNodos, int cantidadDeParticiones) {
  int cantidadDeNodosPorParticion = cantidadDeNodos / cantidadDeParticiones;
  int cantidadDeNodosUltimaParticion = cantidadDeNodosPorParticion;
  if(cantidadDeNodos % cantidadDeParticiones > 0) {
    cantidadDeNodosUltimaParticion += cantidadDeNodos % cantidadDeParticiones;
  }
  int nodoActual = 1;
  // Genero todas las particiones menos la ultima
  for(int i = 0; i < cantidadDeParticiones - 1; i++) {
    nodoActual = generarParticion(archivo, cantidadDeNodosPorParticion, nodoActual);
  }
  // Genero la ultimaparticion
  generarParticion(archivo, cantidadDeNodosUltimaParticion, nodoActual);
}

int main(int argc, char **argv) {
  int cantidadDeNodos = atoi(argv[1]);
  double densidadDeAristas = atof(argv[2]);
  int cantidadDeParticiones = atoi(argv[3]);
  char* nombreDeArchivo = argv[4];

  string outputfile = "instancia.col";
  ofstream archivo(nombreDeArchivo);

  archivo << "c Instancia generada aleatoriamente" << endl;
  archivo << "p edge " << cantidadDeNodos << " " << 5 << " " << cantidadDeParticiones << endl;

  generarParticiones(archivo, cantidadDeNodos, cantidadDeParticiones);
  int cantidadDeAristas = generarAristas(archivo, cantidadDeNodos, densidadDeAristas);

  archivo.close();
  return 0;
}


