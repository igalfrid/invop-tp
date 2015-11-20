#include <ilcplex/ilocplex.h>
#include <ilcplex/cplex.h>
ILOSTLBEGIN
#include <string>
#include <vector>
#include <list>
#include "Grafo.h"

#define TOL 1E-05
#define IT_PLANOS_DE_CORTE 3

void generarVariablesParaCombinacionesDeLosNodos(CPXENVptr env, CPXLPptr lp, int cantidadDeNodos, int longitudNodo) {
  // Agrego las variables por todos cada combinacion de nodo
  int status;
  int longitudNombreVariable = 2 + 2*longitudNodo;
  double ub[cantidadDeNodos][cantidadDeNodos], lb[cantidadDeNodos][cantidadDeNodos], objfun[cantidadDeNodos][cantidadDeNodos];
  char *colnames[cantidadDeNodos][cantidadDeNodos], xctype[cantidadDeNodos][cantidadDeNodos];
  for(int i = 0; i < cantidadDeNodos; i++) {
    for(int j = 0; j < cantidadDeNodos; j++) {
      ub[i][j] = 1;
      lb[i][j] = 0.0;
      colnames[i][j] = new char[longitudNombreVariable];
      sprintf(colnames[i][j],"x%d_%d", i+1, j+1);
      objfun[i][j] = 0;
      xctype[i][j] = 'B';
    }
    status = CPXnewcols(env, lp, cantidadDeNodos, objfun[i], lb[i], ub[i], xctype[i], colnames[i]);
    if (status) {
      cerr << "Problema agregando las variables por cada combinacion de nodo" << endl;
      exit(1);
    }
  }
  for(int i = 0; i < cantidadDeNodos; i++) {
    for(int j = 0; j < cantidadDeNodos; j++) {
      delete[] colnames[i][j];
    }
  }
}

void generarVariablesParaCadaColor(CPXENVptr env, CPXLPptr lp, int cantidadDeNodos, int longitudNodo) {
  // Agrego las columnas por cada color
  int longitudNombreVariable = 1 + longitudNodo;
  int status;
  double ubw[cantidadDeNodos], lbw[cantidadDeNodos], objfunw[cantidadDeNodos];
  char *colnamesw[cantidadDeNodos], xctypew[cantidadDeNodos];
  for(int i = 0; i < cantidadDeNodos; i++) {
    ubw[i] = 1;
    lbw[i] = 0.0;
    colnamesw[i] = new char[longitudNombreVariable];
    sprintf(colnamesw[i],"w%d", i+1);
    objfunw[i] = 1;
    xctypew[i] = 'B';
  }
  status = CPXnewcols(env, lp, cantidadDeNodos, objfunw, lbw, ubw, xctypew, colnamesw);
  if (status) {
    cerr << "Problema agregando las variables por cada color de nodo" << endl;
    exit(1);
  }
  for(int i = 0; i < cantidadDeNodos; i++) {
    delete[] colnamesw[i];
  }
}

void generarRestriccionesWp(CPXENVptr env, CPXLPptr lp, int cantidadDeNodos, int longitudNodo) {
  //Agrego las restricciones wp que me dicen que si una variable p esta seteada, entonces la w tiene que estar seteada
  int longitudNombreVariable = 4 + 2*longitudNodo;
  for(int i = 0; i<cantidadDeNodos; i++) {
    // Generamos de a una las restricciones.
    // Estos valores indican:
    // ccnt = numero nuevo de columnas en las restricciones.
    // rcnt = cuantas restricciones se estan agregando.
    // nzcnt = # de coeficientes != 0 a ser agregados a la matriz. Solo se pasan los valores que no son cero.

    int ccnt = 0, rcnt = cantidadDeNodos, nzcnt = 0;
    char *sense = new char[rcnt]; // Sentido de la desigualdad. 'G' es mayor o igual y 'E' para igualdad.
    double *rhs = new double[rcnt]; // Termino independiente de las restricciones.
    int *matbeg = new int[rcnt]; //Posicion en la que comienza cada restriccion en matind y matval.
    int *matind = new int[rcnt * 2]; // Array con los indices de las variables con coeficientes != 0 en la desigualdad.
    double *matval = new double[rcnt * 2]; // Array que en la posicion i tiene coeficiente ( != 0) de la variable cutind[i] en la restriccion.
    char *rownames[rcnt];

    for(int j = 0; j < cantidadDeNodos; j++) {
      sense[j] = 'L'; // Son por menor o igual
      rhs[j] = 0.0; // El termino independiente siempre es 1

      matbeg[j] = nzcnt; // ASI ESTABA EN EL EJEMPLO, NO ENTIENDO BIEN QUE ES
      matval[nzcnt] = 1; // El valor del coeficiente de cada termino es 1
      matval[nzcnt + 1] = -1; // El valor del coeficiente de cada w es -1

      matind[nzcnt] = i * cantidadDeNodos + j;
      matind[nzcnt + 1] = cantidadDeNodos * cantidadDeNodos + j;

      rownames[j] = new char[longitudNombreVariable];
      sprintf(rownames[j],"wp_%d_%d", i+1, j+1);

      nzcnt+=2;
    }
    // Esta rutina agrega la restriccion al lp.
    int status = CPXaddrows(env, lp, ccnt, rcnt, nzcnt, rhs, sense, matbeg, matind, matval, NULL, rownames);
    if (status) {
        cerr << "Problema agregando las restricciones wp" << endl;
        exit(1);
    }
  }
}

void generarRestriccionPorParticion(CPXENVptr env, CPXLPptr lp, Particion particion, int cantidadDeNodos, int numeroDeParticion) {
  // Agregamos la restriccion
  int cantidadDeNodosEnLaParticion = particion.getCantidadDeNodos();
  int nodo;
  int ccnt = 0, rcnt = 1, nzcnt = 0;
  char *sense = new char[rcnt]; // Sentido de la desigualdad. 'G' es mayor o igual y 'E' para igualdad.
  double *rhs = new double[rcnt]; // Termino independiente de las restricciones.
  int *matbeg = new int[rcnt]; //Posicion en la que comienza cada restriccion en matind y matval.
  int *matind = new int[cantidadDeNodosEnLaParticion * cantidadDeNodos]; // Array con los indices de las variables con coeficientes != 0 en la desigualdad.
  double *matval = new double[cantidadDeNodosEnLaParticion * cantidadDeNodos]; // Array que en la posicion i tiene coeficiente ( != 0) de la variable cutind[i] en la restriccion.
  char *rownames[rcnt];

  sense[0] = 'E'; // Es por igualdad
  rhs[0] = 1; // El termino independiente es 1
  matbeg[0] = nzcnt; // ASI ESTABA EN EL EJEMPLO, NO ENTIENDO BIEN QUE ES
  int longitudNombreVariable = 2 + ceil(log10(numeroDeParticion));
  rownames[0] = new char[longitudNombreVariable];
  sprintf(rownames[0],"v_%d", numeroDeParticion);


  list<int> nodos = particion.getNodos();
  list<int>::iterator it;
  for (it=nodos.begin(); it!=nodos.end(); ++it) {
    int nodo = *it;
    for(int j = 0; j < cantidadDeNodos; j++) {
      matval[nzcnt] = 1; // El valor del coeficiente de cada termino es 1
      matind[nzcnt] = (nodo - 1) * cantidadDeNodos + j;
      nzcnt++;
    }
  }

  //AGREGAR ACA LAS RESTRICCIONES POR LA PARTICION
  int status = CPXaddrows(env, lp, ccnt, rcnt, nzcnt, rhs, sense, matbeg, matind, matval, NULL, rownames);
  if (status) {
    cerr << "Problema agregando la restriccion para la particion: " << numeroDeParticion << endl;
    exit(1);
  }

  delete[] rhs;
  delete[] matbeg;
  delete[] matind;
  delete[] matval;
  for(int i = 0; i < rcnt; i++) {
    delete[] rownames[i];
  }
}

void generarRestriccionesColParaArista(CPXENVptr env, CPXLPptr lp, int cantidadDeNodos, int longitudNodo, int origen, int destino) {
  // Genero las restricciones col que me dicen que los nodos adyacentes no pueden tener el mismo color

  // Generamos de a una las restricciones.
  // Estos valores indican:
  // ccnt = numero nuevo de columnas en las restricciones.
  // rcnt = cuantas restricciones se estan agregando.
  // nzcnt = # de coeficientes != 0 a ser agregados a la matriz. Solo se pasan los valores que no son cero.
  int longitudNombreVariable = 6 + 3*longitudNodo;
  int ccnt = 0, rcnt = cantidadDeNodos, nzcnt = 0;
  char *sense = new char[rcnt]; // Sentido de la desigualdad. 'G' es mayor o igual y 'E' para igualdad.
  double *rhs = new double[rcnt]; // Termino independiente de las restricciones.
  int *matbeg = new int[rcnt]; //Posicion en la que comienza cada restriccion en matind y matval.
  int *matind = new int[rcnt * 2]; // Array con los indices de las variables con coeficientes != 0 en la desigualdad.
  double *matval = new double[rcnt * 2]; // Array que en la posicion i tiene coeficiente ( != 0) de la variable cutind[i] en la restriccion.
  char *rownames[rcnt];

  for(int i = 0; i < rcnt; i++) {
    sense[i] = 'L'; // Son por menor o igual
    rhs[i] = 1.0; // El termino independiente siempre es 1

    matbeg[i] = nzcnt; // ASI ESTABA EN EL EJEMPLO, NO ENTIENDO BIEN QUE ES
    matval[nzcnt] = 1; // El valor del coeficiente de cada termino es 1
    matval[nzcnt + 1] = 1; // El valor del coeficiente de cada termino es 1

    matind[nzcnt] = (origen-1) * cantidadDeNodos + i;
    matind[nzcnt + 1] = (destino-1) * cantidadDeNodos + i;

    rownames[i] = new char[longitudNombreVariable];
    sprintf(rownames[i],"col_%d_%d_%d", origen, destino, i+1);

    nzcnt+=2;
  }

  // Esta rutina agrega la restriccion al lp.
  int status = CPXaddrows(env, lp, ccnt, rcnt, nzcnt, rhs, sense, matbeg, matind, matval, NULL, rownames);
  if (status) {
    cerr << "Problema agregando las restricciones para la arista (" << origen << "," << destino << ")" << endl;
    exit(1);
  }

  delete[] rhs;
  delete[] matbeg;
  delete[] matind;
  delete[] matval;
  for(int i = 0; i < rcnt; i++) {
    delete[] rownames[i];
  }
}

void generarLP(CPXENVptr env, CPXLPptr lp, Grafo grafo) {
  int cantidadDeNodos = grafo.getCantidadDeNodos();
  int longitudNombreVariable;
  int status;

  // Generamos las variables
  int longitudNodo = ceil(log10(cantidadDeNodos));
  generarVariablesParaCombinacionesDeLosNodos(env, lp, cantidadDeNodos, longitudNodo);
  generarVariablesParaCadaColor(env, lp, cantidadDeNodos, longitudNodo);

  // Generamos las restricciones WP
  generarRestriccionesWp(env, lp, cantidadDeNodos, longitudNodo);


  // Generamos las restricciones para las aristas
  list<Arista> aristas = grafo.getAristas();
  list<Arista>::iterator itAristas;
  for (itAristas=aristas.begin(); itAristas!=aristas.end(); ++itAristas) {
    generarRestriccionesColParaArista(env, lp, cantidadDeNodos, longitudNodo, itAristas->getOrigen(), itAristas->getDestino());
  }


  // Generamos las restricciones para las particiones
  list<Particion> particiones = grafo.getParticiones();
  list<Particion>::iterator itParticiones;
  int numeroDeParticion = 1;
  for (itParticiones=particiones.begin(); itParticiones!=particiones.end(); ++itParticiones) {
    generarRestriccionPorParticion(env, lp, *itParticiones, cantidadDeNodos, numeroDeParticion);
    numeroDeParticion++;
  }

  if (status) {
    cerr << "Problema escribiendo modelo" << endl;
    exit(1);
  }
}

void setearParametrosParaCplex(CPXENVptr env) {
  int status;

  // Para desactivar la salida poern CPX_OFF.
  status = CPXsetintparam(env, CPX_PARAM_SCRIND, CPX_ON);

  if (status) {
    cerr << "Problema seteando SCRIND" << endl;
    exit(1);
  }
  // Por ahora no va a ser necesario, pero mas adelante si. Setea el tiempo
  // limite de ejecucion.
  status = CPXsetdblparam(env, CPX_PARAM_TILIM, 2*3600);

  if (status) {
    cerr << "Problema seteando el tiempo limite" << endl;
    exit(1);
  }
}

void setearParametrosDeCPLEXParaBranchAndBoundPuro(CPXENVptr env) {
  //Para que haga Branch & Bound:
  int status = CPXsetintparam(env, CPX_PARAM_MIPSEARCH, CPX_MIPSEARCH_TRADITIONAL);
  if (status) {
    cerr << "Problema seteando el parametro CPX_PARAM_MIPSEARCH en CPX_MIPSEARCH_TRADITIONAL" << endl;
    exit(1);
  }

  //Para facilitar la comparación evitamos paralelismo:
  status = CPXsetintparam(env, CPX_PARAM_THREADS, 1);
  if (status) {
    cerr << "Problema seteando el parametro CPX_PARAM_THREADS en 1" << endl;
    exit(1);
  }

  //Para que no se adicionen planos de corte:
  status = CPXsetintparam(env,CPX_PARAM_EACHCUTLIM, 0);
  if (status) {
    cerr << "Problema seteando el parametro CPX_PARAM_EACHCUTLIM en 0" << endl;
    exit(1);
  }

  status = CPXsetintparam(env, CPX_PARAM_FRACCUTS, -1);
  if (status) {
    cerr << "Problema seteando el parametro CPX_PARAM_FRACCUTS en -1" << endl;
    exit(1);
  }
}

double resolverLP(CPXENVptr env, CPXLPptr lp) {
  // Resolvemos usando CPLEX pero branch and bound
  double inittime, endtime;
  int status;

  // Tomamos el tiempo de resolucion utilizando CPXgettime.
  status = CPXgettime(env, &inittime);

  // Optimizamos el problema.
  status = CPXmipopt(env, lp);

  if (status) {
    cerr << "Problema optimizando CPLEX, status: " << status << endl;
    exit(1);
  }

  status = CPXgettime(env, &endtime);

  if (status) {
    cerr << "Problema obteniendo el tiempo luego de terminar el lp" << endl;
    exit(1);
  }

  // Chequeamos el estado de la solucion.
  int solstat;
  char statstring[510];
  CPXCHARptr p;
  solstat = CPXgetstat(env, lp);
  p = CPXgetstatstring(env, solstat, statstring);
  string statstr(statstring);
  cout << endl << "Resultado de la optimizacion: " << statstring << endl;
  return endtime - inittime;
}

void generarResultados(CPXENVptr env, CPXLPptr lp, int cantidadDeNodos, double tiempoDeCorrida, char* nombreDelArchivoDeSalida) {
  double objval;
  int status = CPXgetobjval(env, lp, &objval);

  if (status) {
    cerr << "Problema obteniendo valor de mejor solucion." << endl;
    exit(1);
  }

  cout << "Datos de la resolucion: " << "\t" << objval << "\t" << tiempoDeCorrida << endl;

  // Tomamos los valores de la solucion y los escribimos a un archivo.
  ofstream solfile(nombreDelArchivoDeSalida);

  // Tomamos los valores de todas las variables. Estan numeradas de 0 a n-1.
  int cantidadDeVariables = CPXgetnumcols(env, lp);
  double *sol = new double[cantidadDeVariables];
  status = CPXgetx(env, lp, sol, 0, cantidadDeVariables - 1);

  if (status) {
    cerr << "Problema obteniendo la solucion del LP." << endl;
    exit(1);
  }

  int longitudNodo = ceil(log10(cantidadDeNodos));
  int longitudNombreVariable = 6 + 3*longitudNodo;
  int storespace=cantidadDeVariables * longitudNombreVariable;
  char * namestore = new char[storespace];
  char ** names = new char* [cantidadDeVariables];
  int sp;
  status = CPXgetcolname(env, lp, names, namestore, storespace, &sp, 0, cantidadDeVariables - 1);

  if (status) {
    cerr << "Problema obteniendo la solucion del LP." << endl;
    exit(1);
  }

  // Solo escribimos las variables distintas de cero (tolerancia, 1E-05).
  solfile << "Tiempo de corrida: " << tiempoDeCorrida << endl;
  solfile << "Valor de la funcion objetivo: " << objval << endl;
  for (int i = 0; i < cantidadDeVariables; i++) {
    if (sol[i] > TOL) {
      solfile << names[i] << " = " << sol[i] << endl;
    }
  }

  delete []names;
  delete []namestore;

  delete [] sol;
  solfile.close();
}

/**
* Lee el archivo de entrada y devuelve el grafo que representa
*/
Grafo leerArchivoDeEntrada(char* nombreDelArchivo) {
  ifstream archivo;
  int cantidadDeNodos;
  int cantidadDeAristas;
  int cantidadDeParticiones;
  int numeroDeParticion = 0;
  int longitudNombreVariable;
  int longitudNodo;
  int status;

  cout << "Nombre del archivo: " << nombreDelArchivo << endl;
  archivo.open(nombreDelArchivo);
  if(archivo.fail()) {
    cout << "No se pudo abrir el archivo: " << nombreDelArchivo << endl;
    exit(1);
  }

  string tipoDeLinea, descarte;
  int origen, destino;

  Grafo grafo;

  while(!archivo.eof()) {
    archivo >> tipoDeLinea;
    if(tipoDeLinea == "p") {
      // Es la linea que describe cantidad de nodos y de aristas
      archivo >> descarte;
      archivo >> cantidadDeNodos;

      grafo.setCantidadDeNodos(cantidadDeNodos);
    }

    else if(tipoDeLinea == "v") {
      // Es una particion
      int cantidadDeNodosEnLaParticion;
      archivo >> cantidadDeNodosEnLaParticion;

      Particion particion;
      int nodo;
      for(int i = 0; i < cantidadDeNodosEnLaParticion; i++) {
        archivo >> nodo;
        particion.agregarNodo(nodo);
      }
      grafo.agregarParticion(particion);
    }

    else if(tipoDeLinea == "e") {
      // Es una arista
      archivo >> origen;
      archivo >> destino;
      grafo.agregarArista(origen, destino);
    }
  }
  archivo.close();

  return grafo;
}

void convertirVariablesLP(CPXENVptr env, CPXLPptr lp, char type) {
  int cantidadDeVariables = CPXgetnumcols(env, lp);
  int * indices = new int[cantidadDeVariables];
  char * types = new char[cantidadDeVariables];

  for(int i = 0; i < cantidadDeVariables; i++) {
    indices[i] = i;
    types[i] = type;
  }

  int status = CPXchgctype (env, lp, cantidadDeVariables, indices, types);

  delete []indices;
  delete []types;

  if(status) {
    cerr << "Error cambiando los tipos de las variables del problema" << endl;
    exit(1);
  }
}

void relajarLP(CPXENVptr env, CPXLPptr lp) {
  convertirVariablesLP(env, lp, CPX_CONTINUOUS);
}

void desRelajarLP(CPXENVptr env, CPXLPptr lp) {
  convertirVariablesLP(env, lp, CPX_BINARY);
}

/**
* Ordena los valores de forma descendiente y los indices los alinea a las posiciones de los valores.
* Utilizo selection sort.
*/
void ordenarDescendientemente(double * valores, int * indices, int cantidad) {
  double valorMaximo;
  int indiceMaximo;
  for(int i=0; i<cantidad; i++) {
    indiceMaximo = 0;
    valorMaximo = 0;
    for(int j=0; j<cantidad; j++) {
      if(valores[j] > valorMaximo) {
        valorMaximo = valores[j];
        indiceMaximo = j;
      }
    }
    swap(valores[i], valores[indiceMaximo]);
    swap(indices[i], indices[indiceMaximo]);
  }
}

/**
* Agrega una restriccion al lp por la suma de los nodos de color j menor o igual al wj
*/
void agregarDesigualdadALP(CPXENVptr env, CPXLPptr lp, list<int> nodos, int colorJ, int cantidadDeNodos, int coeficienteWj, char* nombreDesigualdad) {
  // Estos valores indican:
  // ccnt = numero nuevo de columnas en las restricciones.
  // rcnt = cuantas restricciones se estan agregando.
  // nzcnt = # de coeficientes != 0 a ser agregados a la matriz. Solo se pasan los valores que no son cero.

  int columnasAAgregar = 0;
  int restriccionesAAgregar = 1;
  int coeficientesDistintosDe0 = nodos.size() + 1;

  char *sense = new char[restriccionesAAgregar]; // Sentido de la desigualdad. 'G' es mayor o igual y 'E' para igualdad.
  double *rhs = new double[restriccionesAAgregar]; // Termino independiente de las restricciones.
  int *matbeg = new int[restriccionesAAgregar]; //Posicion en la que comienza cada restriccion en matind y matval.
  int *matind = new int[restriccionesAAgregar * coeficientesDistintosDe0 + 1]; // Array con los indices de las variables con coeficientes != 0 en la desigualdad.
  double *matval = new double[restriccionesAAgregar * coeficientesDistintosDe0 + 1]; // Array que en la posicion i tiene coeficiente ( != 0) de la variable cutind[i] en la restriccion.
  char *rownames[restriccionesAAgregar];

  sense[0] = 'L'; // Son por menor o igual
  rhs[0] = 0.0; // El termino independiente siempre es 0
  rownames[0] = nombreDesigualdad;
  matbeg[0] = 0; // ASI ESTABA EN EL EJEMPLO, NO ENTIENDO BIEN QUE ES

  int i=0;
  list<int>::iterator it;
  for (it=nodos.begin(); it!=nodos.end(); ++it) {
    // El nodo i con el color j es sol[i*cantidadDeNodos + j]
    matind[i] = (*it - 1) * cantidadDeNodos + (colorJ - 1); // El menos 1 es porque los nodos arrancan de 1 y las variables de 0
    matval[i] = 1; // El valor de cada coeficiente de x es 1
    i++;
  }
  // El color j es sol[cantidadDeNodos*cantidadDeNodos + j]
  matind[i] = cantidadDeNodos * cantidadDeNodos + (colorJ - 1);
  matval[i] = coeficienteWj; // El valor del coeficiente de wj

  // Esta rutina agrega la restriccion al lp.
  int status = CPXaddrows(env, lp, columnasAAgregar, restriccionesAAgregar, coeficientesDistintosDe0, rhs, sense, matbeg, matind, matval, NULL, rownames);
  if (status) {
    cerr << "Problema agregando restriccion por desigualdad clique" << endl;
    exit(1);
  }

  delete[] sense;
  delete[] rhs;
  delete[] matbeg;
  delete[] matind;
  delete[] matval;
  delete[] rownames[0];
}

/**
* Dada la solucion para un color ordenada descendentemente busca una clique
* cuya desigualdad sea violada por la solucion y se agrega la restriccion
* correspondiente al LP
*/
int buscarCliqueMaximalQueVioleLaDesigualdadCliqueYAgregarla(CPXENVptr env, CPXLPptr lp, double * valorVariablePorColor, int * indices, int colorJ, double valorWj, Grafo grafo, int numeroDeDesigualdadClique){
  int cantidadDeNodos = grafo.getCantidadDeNodos();
  int nodo;
  int indicePrimerNodoNoUsado = 0;
  bool encontreCliqueNueva = false;
  int coeficienteWj = -1;
  list<int> clique;
  while(indicePrimerNodoNoUsado < cantidadDeNodos && !encontreCliqueNueva) {
    clique.clear();

    clique.push_back(indices[indicePrimerNodoNoUsado]);
    double sumaDeLaClique = valorVariablePorColor[0];
    for(int i=1; i<cantidadDeNodos; i++) {
      nodo = indices[i];
      if(grafo.esAdyacenteATodos(clique, nodo)) {
        clique.push_back(nodo);
        sumaDeLaClique += valorVariablePorColor[i];
      }
    }

    if(sumaDeLaClique > valorWj) {
      // La desigualdad de esta clique viola a la solucion actual
      // Genero nueva restriccion
      encontreCliqueNueva = true;
      int longitudNombreDeDesigualdad = 2 + ceil(log10(numeroDeDesigualdadClique));
      char* nombreDeLaDesigualdad = new char[longitudNombreDeDesigualdad];
      sprintf(nombreDeLaDesigualdad,"k_%d", numeroDeDesigualdadClique);
      agregarDesigualdadALP(env, lp, clique, colorJ, cantidadDeNodos, coeficienteWj, nombreDeLaDesigualdad);
      numeroDeDesigualdadClique++;
    }

    indicePrimerNodoNoUsado++;
  }

  return numeroDeDesigualdadClique;
}

/**
* Auxiliar sacar a un Util
*/
void desplazarADerechaUnaPosicionAPartirDe(int * items, int desde, int cantidad) {
  for(int i=cantidad-1; i >= desde; i--) {
    items[i+1] = items[i];
  }
}

/**
* Auxiliar sacar a un Util
*/
void desplazarAIzquierdaUnaPosicionAPartirDe(int * items, int desde, int cantidad) {
  for(int i=desde; i<cantidad; i++) {
    items[i-1] = items[i];
  }
}
/**
* Auxiliar sacar a un Util
*/
int indiceEn(int * items, int item, int cantidad) {
  for(int i=0; i<cantidad; i++) {
    if(items[i] == item) {
      return i;
    }
  }
  return -1;
}

/**
* Dada la solucion para un color ordenada descendentemente busca un agujero
* impar cuya desigualdad sea violada por la solucion y se agrega la restriccion
* correspondiente al LP
*/
int buscarAgujeroImparQueVioleLaDesigualdadAgujeroImparYAgregarla(CPXENVptr env, CPXLPptr lp, double * valorVariablePorColor, int * indices, int colorJ, double valorWj, Grafo grafo, int numeroDeDesigualdadAgujeroImpar){

  int cantidadDeNodos = grafo.getCantidadDeNodos();
  int nodo;
  int indiceEnSolucion;
  double sumaDelAgujero = 0;
  int * agujero = new int[cantidadDeNodos];
  int cantidadDeNodosEnElAgujero = 0;

  // Comienzo el agujero con una arista que tenga al nodo de mayor valor posible
  int j = 0;
  int i;
  bool encontreArista = false;
  while(j < cantidadDeNodos && !encontreArista) {
    nodo = indices[0];
    i = j + 1;
    while(i < cantidadDeNodos && !encontreArista) {
      if(grafo.sonAdyacentes(nodo, indices[i])) {
        // Si son adyacentes entonces tengo una arista, agrego los primeros nodos
        agujero[cantidadDeNodosEnElAgujero] = indices[0];
        sumaDelAgujero += valorVariablePorColor[0];
        cantidadDeNodosEnElAgujero++;
        agujero[cantidadDeNodosEnElAgujero] = indices[i];
        sumaDelAgujero += valorVariablePorColor[i];
        cantidadDeNodosEnElAgujero++;
        encontreArista = true;
      }
      i++;
    }
    j++;
  }

  if(!encontreArista) {
    cout << "No encontre arista para comenzar a generar el agujero" << endl;
    return numeroDeDesigualdadAgujeroImpar;
  }
  // Hasta aca solo tenemos dos nodos en el agujero

  // Agrego nodos al agujero buscando entre cada par de nodos si
  // puedo poner un nodo en el medio
  bool agregueNodoAlAgujero;
  int ultimoNodoAgregado;
  double valorUltimoNodoAgregado;
  list<int> nodos;
  i = 1;
  //while (i < cantidadDeNodosEnElAgujero) {
    j = i;
    agregueNodoAlAgujero = false;
    while (j < cantidadDeNodosEnElAgujero){// && !agregueNodoAlAgujero) {
      nodos.clear();
      nodos.push_back(agujero[j]);
      nodos.push_back(agujero[j-1]);

      int k = 0;
      //nodo = 1;
      //while(nodo <= cantidadDeNodos && !agregueNodoAlAgujero) {
      while(k < cantidadDeNodos){// && !agregueNodoAlAgujero) {
        nodo = indices[k];
        if(grafo.esAdyacenteATodos(nodos, nodo)) {
          // Si el nodo es adyacente a los dos entonces lo puedo meter en el medio
          if (indiceEn(agujero, nodo, cantidadDeNodosEnElAgujero) == -1) {
            // El nodo no pertenece al agujero
            desplazarADerechaUnaPosicionAPartirDe(agujero, j, cantidadDeNodosEnElAgujero);
            agujero[j] = nodo;
            indiceEnSolucion = indiceEn(indices, nodo, cantidadDeNodos);
            valorUltimoNodoAgregado = valorVariablePorColor[indiceEnSolucion];
            sumaDelAgujero += valorUltimoNodoAgregado;
            ultimoNodoAgregado = nodo;
            cantidadDeNodosEnElAgujero++;
            agregueNodoAlAgujero = true;
            i = 0;
          }
        }
        k++;
      }
      j++;
    }
  //  i++;
  //}

  if(cantidadDeNodosEnElAgujero < 3) {
    cout << "No encontre un agujero" << endl;
    return numeroDeDesigualdadAgujeroImpar;
  }

  if(cantidadDeNodosEnElAgujero % 2 == 0) {
    // Quiere decir que es un agujero par, debo sacar un nodo
    int indiceEnAgujero = indiceEn(agujero, ultimoNodoAgregado, cantidadDeNodosEnElAgujero);
    // Saco el ultimo nodo que agregue
    desplazarAIzquierdaUnaPosicionAPartirDe(agujero, indiceEnAgujero+1, cantidadDeNodosEnElAgujero);
    cantidadDeNodosEnElAgujero--;
    sumaDelAgujero -= valorUltimoNodoAgregado;
  }

  int coeficienteWj = (cantidadDeNodosEnElAgujero - 1) / 2;
  if(sumaDelAgujero > coeficienteWj * valorWj) {
    // La desigualdad de este agujero impar viola a la solucion actual
    // Genero nueva restriccion
    list<int> agujeroImpar;
    for(int i=0; i<cantidadDeNodosEnElAgujero; i++) {
      agujeroImpar.push_back(agujero[i]);
    }
    int longitudNombreDeDesigualdad = 2 + ceil(log10(numeroDeDesigualdadAgujeroImpar));
    char* nombreDeLaDesigualdad = new char[longitudNombreDeDesigualdad];
    sprintf(nombreDeLaDesigualdad,"a_%d", numeroDeDesigualdadAgujeroImpar);

    agregarDesigualdadALP(env, lp, agujeroImpar, colorJ, cantidadDeNodos, -coeficienteWj, nombreDeLaDesigualdad);
    numeroDeDesigualdadAgujeroImpar++;
  }

  return numeroDeDesigualdadAgujeroImpar;
}

void agregarPlanosDeCorte(CPXENVptr env, CPXLPptr lp, Grafo grafo, int iteracionesPlanosDeCorte) {
  int cantidadDeNodos = grafo.getCantidadDeNodos();
  int cantidadDeVariables = CPXgetnumcols(env, lp);
  double *solucion = new double[cantidadDeVariables];
  double * valorVariablePorColor = new double[cantidadDeNodos];
  int * indices = new int[cantidadDeNodos];
  int numeroDeDesigualdadClique = 1;
  int numeroDeDesigualdadAgujeroImpar = 1;
  bool agregueClique, agregueAgujero;
  for(int i = 0; i < iteracionesPlanosDeCorte; i++) {
    resolverLP(env, lp);
    CPXgetx(env, lp, solucion, 0, cantidadDeVariables - 1);
    char *archivoResultado = new char[27 + 2];
    sprintf(archivoResultado,"resultados_temporales_%d.sol", i);
    generarResultados(env, lp, cantidadDeNodos, 10.0, archivoResultado);

    // Para cada color busco desigualdades que violen la solucion obtenida
    for(int j = 0; j < cantidadDeNodos; j++) {
      // Para cada color buscamos una clique maximal y agregamos la restriccion
      // El nodo i con el color j es sol[i*cantidadDeNodos + j]
      // El color j es sol[cantidadDeNodos*cantidadDeNodos + j]
      for(int i = 0; i < cantidadDeNodos; i++) {
        valorVariablePorColor[i] = solucion[i * cantidadDeNodos + j];
        indices[i] = i+1;
      }
      double valorWj = solucion[cantidadDeNodos*cantidadDeNodos + j];
      int colorJ = j + 1;
      ordenarDescendientemente(valorVariablePorColor, indices, cantidadDeNodos);
      numeroDeDesigualdadClique = buscarCliqueMaximalQueVioleLaDesigualdadCliqueYAgregarla(env, lp, valorVariablePorColor, indices, colorJ, valorWj, grafo, numeroDeDesigualdadClique);
      numeroDeDesigualdadAgujeroImpar = buscarAgujeroImparQueVioleLaDesigualdadAgujeroImparYAgregarla(env, lp, valorVariablePorColor, indices, colorJ, valorWj, grafo, numeroDeDesigualdadAgujeroImpar);
    }
  }
  delete [] valorVariablePorColor;
  delete [] indices;
  delete[] solucion;
}

int main(int argc, char **argv) {
  // Leemos el archivo y obtenemos el grafo
  char * nombreDelArchivoDeEntrada = argv[1];
  char * nombreDelArchivoDeSalida = argv[2];
  int iteracionesPlanosDeCorte = 3; //Por default hace 3 iteraciones
  if(argv[3] != NULL) {
    iteracionesPlanosDeCorte = atoi(argv[3]);
  }
  cout << "Se utilizan " << iteracionesPlanosDeCorte << " iteraciones de planos de corte" << endl;
  Grafo grafo = leerArchivoDeEntrada(nombreDelArchivoDeEntrada);
  cout << "El grafo tiene " << grafo.getCantidadDeNodos() << " nodos" << endl;
  cout << "El grafo tiene " << grafo.getCantidadDeAristas() << " aristas" << endl;
  cout << "El grafo tiene " << grafo.getCantidadDeParticiones() << " particiones" << endl;

  // Creamos el LP asociado al problema
  int status;
  CPXENVptr env; // Puntero al entorno.
  CPXLPptr lp; // Puntero al LP

  // Creo el entorno.
  env = CPXopenCPLEX(&status);
  if (env == NULL) {
    cerr << "Error creando el entorno" << endl;
    exit(1);
  }

  // Creo el LP.
  lp = CPXcreateprob(env, &status, nombreDelArchivoDeEntrada);
  if (lp == NULL) {
    cerr << "Error creando el LP" << endl;
    exit(1);
  }

  setearParametrosParaCplex(env);
  setearParametrosDeCPLEXParaBranchAndBoundPuro(env);

  // Ahora si generamos el LP a partir del grafo
  generarLP(env, lp, grafo);

  double inittime, endtime;

  // Tomamos el tiempo de resolucion utilizando CPXgettime.
  status = CPXgettime(env, &inittime);

  // Relajo el LP
  relajarLP(env, lp);

  // Agregamos los planos de corte
  agregarPlanosDeCorte(env, lp, grafo, iteracionesPlanosDeCorte);

  // Quito la relajacion
  desRelajarLP(env, lp);

  // Escribimos el LP
  CPXwriteprob(env, lp, "modelo.lp", NULL);

  double tiempoDeCorrida = resolverLP(env, lp);

  // Tomamos el tiempo de resolucion utilizando CPXgettime.
  status = CPXgettime(env, &endtime);

  cout << "El tiempo total de corrida fue: " << endtime - inittime << endl;

  generarResultados(env, lp, grafo.getCantidadDeNodos(), tiempoDeCorrida, nombreDelArchivoDeSalida);

  return 0;
}
