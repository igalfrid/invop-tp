#include <ilcplex/ilocplex.h>
#include <ilcplex/cplex.h>
ILOSTLBEGIN
#include <string>
#include <vector>
#include <list>

using namespace std;

list< pair<int,int> > generarAristas(int cantidadDeNodos, double densidadDeAristas) {
  list< pair<int,int> > aristas;
  int cantidadDeAristas = 0;
  for(int i = 1; i <= cantidadDeNodos; i++) {
    for(int j = i + 1; j <= cantidadDeNodos; j++) {
      double random = ((double) rand() / (RAND_MAX));
      if(random <= densidadDeAristas) {
        pair<int,int> arista = make_pair(i, j);
        aristas.push_back(arista);
      }
    }
  }
  return aristas;
}

void generarParticiones(ofstream &archivo, int cantidadDeNodos, int cantidadDeParticiones) {
  // Creo las n particiones
  list<int> particiones[cantidadDeNodos];

  // Recorro los nodos y coloco a cada nodo en la particion que le corresponde
  for(int i=1; i<=cantidadDeNodos; i++) {
    int particion = rand() % cantidadDeParticiones;
    particiones[particion].push_back(i);
  }

  // Escrivo las particiones en el archivo
  for(int i=0; i<cantidadDeParticiones; i++) {
    archivo << "v " << particiones[i].size();
    list<int>::iterator it;
    for (it=particiones[i].begin(); it!=particiones[i].end(); ++it) {
      archivo << " " << *it;
    }
    archivo << endl;
  }
}

void escribirAristas(ofstream &archivo, list< pair<int,int> > aristas) {
  list< pair<int,int> >::iterator it;
  for (it=aristas.begin(); it!=aristas.end(); ++it) {
    archivo << "e " << it->first << " " << it->second << endl;
  }
}

int main(int argc, char **argv) {
  int cantidadDeNodos = atoi(argv[1]);
  double densidadDeAristas = atof(argv[2]);
  int cantidadDeParticiones = atoi(argv[3]);
  char* nombreDeArchivo = argv[4];

  // Genero las aristas
  list< pair<int,int> > aristas = generarAristas(cantidadDeNodos, densidadDeAristas);

  // Comienzo a escribir el archivo
  ofstream archivo(nombreDeArchivo);
  archivo << "c Instancia generada aleatoriamente" << endl;
  archivo << "p edge " << cantidadDeNodos << " " << aristas.size() << " " << cantidadDeParticiones << endl;

  // Genero las particiones
  generarParticiones(archivo, cantidadDeNodos, cantidadDeParticiones);

  // Escribo aristas
  escribirAristas(archivo, aristas);

  // Cierro el archivo
  archivo.close();

  return 0;
}


