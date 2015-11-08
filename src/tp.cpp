#include <ilcplex/ilocplex.h>
#include <ilcplex/cplex.h>
ILOSTLBEGIN
#include <string>
#include <vector>

#define TOL 1E-05
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

void generarRestriccionPorParticion(CPXENVptr env, CPXLPptr lp, ifstream &archivo, int cantidadDeNodos, int cantidadDeNodosEnLaParticion, int numeroDeParticion) {
  // Agregamos la restriccion
  cout << "Particion: ";
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

  for(int i = 0; i < cantidadDeNodosEnLaParticion; i++) {
    archivo >> nodo;
    cout << nodo << " ";

    for(int j = 0; j < cantidadDeNodos; j++) {
      matval[nzcnt] = 1; // El valor del coeficiente de cada termino es 1
      matind[nzcnt] = (nodo - 1) * cantidadDeNodos + j;
      nzcnt++;
    }
  }
  cout << endl;

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

void generarLP(CPXENVptr env, CPXLPptr lp, int* cantidadDeNodosDelProblema, char* nombreDelArchivo) {
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

  // Creo el entorno.
  env = CPXopenCPLEX(&status);

  if (env == NULL) {
    cerr << "Error creando el entorno" << endl;
    exit(1);
  }

  // Creo el LP.
  lp = CPXcreateprob(env, &status, nombreDelArchivo);

  if (lp == NULL) {
    cerr << "Error creando el LP" << endl;
    exit(1);
  }

  string tipoDeLinea, descarte;
  int origen, destino;

  while(!archivo.eof()) {
    archivo >> tipoDeLinea;
    if(tipoDeLinea == "p") {
      //Es la linea que describe cantidad de nodos y de aristas
      archivo >> descarte;
      archivo >> cantidadDeNodos;
      archivo >> cantidadDeAristas;
      archivo >> cantidadDeParticiones;
      cout << "Cantidad de nodos: " << cantidadDeNodos << endl;
      cout << "Cantidad de aristas: " << cantidadDeAristas << endl;
      cout << "Cantidad de particiones: " << cantidadDeParticiones << endl;
      longitudNodo = ceil(log10(cantidadDeNodos));

      generarVariablesParaCombinacionesDeLosNodos(env, lp, cantidadDeNodos, longitudNodo);
      generarVariablesParaCadaColor(env, lp, cantidadDeNodos, longitudNodo);
      generarRestriccionesWp(env, lp, cantidadDeNodos, longitudNodo);
    }

    else if(tipoDeLinea == "v") {
      // Es una particion
      numeroDeParticion++;
      int cantidadDeNodosEnLaParticion;
      archivo >> cantidadDeNodosEnLaParticion;

      generarRestriccionPorParticion(env, lp, archivo, cantidadDeNodos, cantidadDeNodosEnLaParticion, numeroDeParticion);
    }

    else if(tipoDeLinea == "e" && cantidadDeAristas > 0) {
      // Es una arista
      cantidadDeAristas--;
      archivo >> origen;
      archivo >> destino;
      cout << "Arista (" << origen << "," << destino  << ")" << endl;

      generarRestriccionesColParaArista(env, lp, cantidadDeNodos, longitudNodo, origen, destino);
    }
  }
  archivo.close();

   // Escribimos el problema a un archivo .lp.
  status = CPXwriteprob(env, lp, "tp.lp", NULL);

  if (status) {
    cerr << "Problema escribiendo modelo" << endl;
    exit(1);
  }

  *cantidadDeNodosDelProblema = cantidadDeNodos;
}

void setearParametrosDeCPLEXParaBranchAndBoundPuro(CPXENVptr env) {
  int status;

  // Para desactivar la salida poern CPX_OFF.
  status = CPXsetintparam(env, CPX_PARAM_SCRIND, CPX_ON);

  if (status) {
    cerr << "Problema seteando SCRIND" << endl;
    exit(1);
  }
  // Por ahora no va a ser necesario, pero mas adelante si. Setea el tiempo
  // limite de ejecucion.
  status = CPXsetdblparam(env, CPX_PARAM_TILIM, 3600);

  if (status) {
    cerr << "Problema seteando el tiempo limite" << endl;
    exit(1);
  }

  //Para que haga Branch & Bound:
  status = CPXsetintparam(env, CPX_PARAM_MIPSEARCH, CPX_MIPSEARCH_TRADITIONAL);
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
  status = CPXlpopt(env, lp);

  status = CPXgettime(env, &endtime);

  if (status) {
    cerr << "Problema optimizando CPLEX" << endl;
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
  if(solstat!=CPX_STAT_OPTIMAL){
    cerr << "La solucion no fue optima." << endl;
    exit(1);
  }

  return endtime - inittime;
}

void generarResultados(CPXENVptr env, CPXLPptr lp, int cantidadDeNodos, double tiempoDeCorrida) {
  double objval;
  int status = CPXgetobjval(env, lp, &objval);

  if (status) {
    cerr << "Problema obteniendo valor de mejor solucion." << endl;
    exit(1);
  }

  cout << "Datos de la resolucion: " << "\t" << objval << "\t" << tiempoDeCorrida << endl;

  // Tomamos los valores de la solucion y los escribimos a un archivo.
  std::string outputfile = "tp.sol";
  ofstream solfile(outputfile.c_str());


  // Tomamos los valores de todas las variables. Estan numeradas de 0 a n-1.
  int cantidadDeVariables = cantidadDeNodos * (cantidadDeNodos + 1);
  double *sol = new double[cantidadDeVariables];
  status = CPXgetx(env, lp, sol, 0, cantidadDeVariables - 1);

  if (status) {
    cerr << "Problema obteniendo la solucion del LP." << endl;
    exit(1);
  }


  // Solo escribimos las variables distintas de cero (tolerancia, 1E-05).
  solfile << "Tiempo de corrida: " << tiempoDeCorrida << endl;
  solfile << "Valor de la funcion objetivo: " << objval << endl;
  for (int i = 0; i < cantidadDeVariables; i++) {
    if (sol[i] > TOL) {
      solfile << "x_" << i << " = " << sol[i] << endl;
    }
  }


  delete [] sol;
  solfile.close();
}

int main(int argc, char **argv) {
  //Leemos el archivo
  char * nombreDelArchivo = argv[1];
  int cantidadDeNodos;

  // Genero el problema de cplex.
  int status;
  CPXENVptr env; // Puntero al entorno.
  CPXLPptr lp; // Puntero al LP

  generarLP(env, lp, &cantidadDeNodos, nombreDelArchivo);

  setearParametrosDeCPLEXParaBranchAndBoundPuro(env);

  if (status) {
    cerr << "Problema seteando CPX_PARAM_SCRIND" << endl;
    exit(1);
  }
  double tiempoDeCorrida = resolverLP(env, lp);

  generarResultados(env, lp, cantidadDeNodos, tiempoDeCorrida);

  return 0;
}


