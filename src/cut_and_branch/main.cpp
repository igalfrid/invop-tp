#include <ilcplex/ilocplex.h>
#include <ilcplex/cplex.h>
ILOSTLBEGIN
#include <string>
#include <vector>
#include <list>
#include "Grafo.h"

#define TOL 1E-05
#define IT_PLANOS_DE_CORTE 3

typedef struct t_ctx {
  CPXENVptr env;
  CPXLPptr lp;

  int nodos;
  int colores;

  // xind devuelve el indice de la variable xnc (pintar el nodo n con el color
  // c). Ambos empiezan por 0
  inline int xind(int n, int c) {
    // la variable xij esta en la posicion (c * colores) + n
    return nodos * c + n;
  }

  // wind devuelve el indice de la variable wc (se usa el color c). Empieza por
  // 0.
  inline int wind(int c) {
    // la variable wj esta en la posicion (nodos * colores) + j
    return nodos * colores + c;
  }

} lpcontext;

/*
   agregarVariablesParaNodos agrega una variable para cada combinacion (nodo,
   color)
   estas son variables booleanas, que valen 1 si el nodo n fue pintado con el
   color c.
   Las variables se agregan en el siguiente orden:
    - x11, x21, x31, ..., xn1, x12, x22, ..., xn2, ..., x1m, x2m, ..., xnm
   Entonces, la variable xij va a tener indice (nodos * i) + j
*/
void agregarVariablesParaNodos(lpcontext &ctx) {
  int nodos = ctx.nodos;
  int colores = ctx.colores;

  vector<double> objective(nodos, 0.0);
  vector<double> lowerbound(nodos, 0.0);
  vector<double> upperbound(nodos, 1.0);
  vector<char> ctype(nodos, CPX_BINARY);

  // Para cada color, agregamos una variable por cada nodo, xn_c que dice que
  // el nodo n fue pintado del color c.
  for (int c = 0; c < colores; c++) {
    vector<char *> colnames(nodos, NULL);
    for (int n = 0; n < nodos; n++) {
      asprintf(&colnames[n], "x%d_%d", n + 1, c + 1);
    }

    /*
       Dado que en c++ los elementos de los vectores se almacenan de forma
       contigua, devolver la direccion del primer elemento es equivalente
       a devolver un puntero al array que lo contiene.

       Esto es una forma facil de convertir de vector<double> a double*
    */
    double *obj = &objective[0];
    double *lb = &lowerbound[0];
    double *ub = &upperbound[0];
    char *ct = &ctype[0];
    char **cnames = &colnames[0];

    int status = CPXnewcols(ctx.env, ctx.lp, nodos, obj, lb, ub, ct, cnames);
    if (status) {
      cerr << "Problema agregando variables de coloreo de nodos" << endl;
      exit(1);
    }

    for (int n = 0; n < nodos; n++) {
      free(colnames[n]);
    }
  }
}

// agregarVariablesParaColores agrega una variable booleana para cada color.
// La variable se pone en 1 si el color es usado para pintar algun nodo o no.
// las variables se agregan en orden ascendente (w1, w2, ..., wm)
// Esto tambien agrega las variables wi a la funcion a optimizar.
void agregarVariablesParaColores(lpcontext &ctx) {
  int colores = ctx.colores;
  // coeficientes que van a tener las variables en la funcion a optimizar.
  vector<double> objective(colores, 1.0);
  vector<double> lowerbound(colores, 0.0);
  vector<double> upperbound(colores, 1.0);
  vector<char> ctype(colores, CPX_BINARY);

  vector<char *> colnames(colores, NULL);

  for (int c = 0; c < colores; c++) {
    asprintf(&colnames[c], "w%d", c + 1);
  }

  /*
    Dado que en c++ los elementos de los vectores se almacenan de forma
    contigua, devolver la direccion del primer elemento es equivalente
    a devolver un puntero al array que lo contiene.

    Esto es una forma facil de convertir de vector<double> a double*
  */
  double *obj = &objective[0];
  double *lb = &lowerbound[0];
  double *ub = &upperbound[0];
  char *ct = &ctype[0];
  char **cnames = &colnames[0];

  int status = CPXnewcols(ctx.env, ctx.lp, colores, obj, lb, ub, ct, cnames);
  if (status) {
    cerr << "Problema agregando las variables por cada color de nodo" << endl;
    exit(1);
  }

  for (int c = 0; c < colores; c++) {
    free(colnames[c]);
  }
}

// agregarRestriccionesDeColor agrega restricciones que dicen que si hay al
// menos un nodo n pintado con el color c, entonces wc tiene que estar pintada.
void agregarRestriccionesDeColor(lpcontext &ctx) {
  int nodos = ctx.nodos;
  int colores = ctx.colores;
  for (int i = 0; i < colores; i++) {
    // las restricciones que agregamos aca son del estilo:
    // x_11 - w1 < 0
    // x_21 - w1 < 0
    // ...
    // x_n1 - w1 < 0
    // para w1 ... wm

    int ccnt = 0;          // numero nuevo de columnas en las restricciones.
    int rcnt = nodos;      // cuantas restricciones se estan agregando.
    int nzcnt = nodos * 2; // cantidad de variables con coefs no nulos.

    // sentido de la desigualdad. 'L' es <=, 'G' >= y 'E' ==
    vector<char> sentido(rcnt, 'L');

    // indep es el termino independiente de cada restriccion.
    vector<double> indep(rcnt, 0.0);

    // begin tiene las pos (de indexes y coefs) en donde empieza c/restriccion
    // por ej: begin[5] es el indice en indexes y coefs de la sexta restriccion
    vector<int> begin(rcnt, 0);

    // indexes tiene los indices de las variables con coefs != 0
    vector<int> indexes(rcnt * 2, 0);

    /* coefs es un array que en la posicion i tiene el coeficiente ( != 0) de
       la variable cutind[i] en una restriccion */
    vector<double> coefs(rcnt * 2);

    vector<char *> rownames(nodos, NULL);

    // empezamos con nzcnt = 0 y le vamos sumando 2 por iteracion, porque
    // por cada nodo vamos a agregar 2 variables a la row (xji - wi <= 0.0)
    for (int j = 0; j < nodos; j++) {
      begin[j] = j * 2;
      coefs[j * 2] = 1;      // El valor del coeficiente de cada xij es 1
      coefs[j * 2 + 1] = -1; // El valor del coeficiente de cada w es -1

      // primer variable: xji
      indexes[j * 2] = ctx.xind(j, i);

      // segunda variable wi
      indexes[j * 2 + 1] = ctx.wind(i);

      asprintf(&rownames[j], "wp_%d_%d", j + 1, i + 1);
    }

    char *sense = &sentido[0];
    double *rhs = &indep[0];
    int *matbeg = &begin[0];
    int *matind = &indexes[0];
    double *matval = &coefs[0];
    char **rn = &rownames[0];

    // Esta rutina agrega la restriccion al lp.
    int status = CPXaddrows(ctx.env, ctx.lp, ccnt, rcnt, nzcnt, rhs, sense,
                            matbeg, matind, matval, NULL, rn);
    if (status) {
      cerr << "Problema agregando las restricciones wp" << endl;
      exit(1);
    }

    for (int j = 0; j < nodos; j++) {
      free(rownames[j]);
    }
  }
}

// agregarRestriccionesPorParticion agrega una restriccion que dice que la
// cantidad de nodos pintados de una particion debe ser exactamente 1.
// Esta restriccion sera del estilo: (suponiendo que los nodos k y f forman la
// particion p)
//  xk1 + ... + xkm + xf1 + ... + xfm = 1
// NOTA: Se asume que los nodos de la particion empiezan por 1 en lugar de 0.
void agregarRestriccionDeParticion(lpcontext &ctx, Particion particion,
                                   int numeroDeParticion) {

  int nodosDeParticion = particion.getCantidadDeNodos();
  int ccnt = 0, rcnt = 1;

  vector<char> sentido(rcnt, 'E');
  vector<double> indep(rcnt, 1.0);
  vector<int> begin(rcnt, 0);
  vector<int> indexes(nodosDeParticion * ctx.colores, 0);

  // el valor de los coeficientes es siempre 1 en este caso.
  vector<double> coefs(nodosDeParticion * ctx.colores, 1.0);
  vector<char *> rownames(rcnt, NULL);

  asprintf(&rownames[0], "v_%d", numeroDeParticion);

  // para cada nodo n de la particion agregamos la variables x que lo usan:
  // xn1 + xn2 + xn3 + ... + xnm
  int nzcnt = 0;
  for (const auto &n : particion.getNodos()) {
    for (int c = 0; c < ctx.colores; c++) {
      indexes[nzcnt] = ctx.xind(n - 1, c);

      nzcnt += 1;
    }
  }

  char *sense = &sentido[0];
  double *rhs = &indep[0];
  int *matbeg = &begin[0];
  int *matind = &indexes[0];
  double *matval = &coefs[0];
  char **rn = &rownames[0];

  int status = CPXaddrows(ctx.env, ctx.lp, ccnt, rcnt, nzcnt, rhs, sense,
                          matbeg, matind, matval, NULL, rn);
  if (status) {
    cerr << "Problema agregando la restriccion para la particion: "
         << numeroDeParticion << endl;
    exit(1);
  }

  for (int i = 0; i < rcnt; i++) {
    free(rownames[i]);
  }
}

// agregarRestriccionDeColoreo agrega restricciones que dicen que los
// nodos adyacentes no pueden tener el mismo color, en caso de estar pintados.
// estas restricciones son de la forma:
// si j, k son dos nodos adyacentes, entonces xj1 + xk1 <= 1 y xj2 + xk2 <= 1
// y asi para todos los colores.
// NOTA: Se asume que origen y destino son nodos que empiezan por 1 en vez de 0.
void agregarRestriccionDeColoreo(lpcontext &ctx, int origen, int destino) {
  int ccnt = 0, rcnt = ctx.colores;

  vector<char> sentido(rcnt, 'L');
  vector<double> indep(rcnt, 1.0);
  vector<int> begin(rcnt, 0);
  vector<int> indexes(rcnt * 2, 0);
  vector<double> coefs(rcnt * 2, 1.0);

  vector<char *> rownames(rcnt, NULL);

  for (int i = 0; i < rcnt; i++) {
    begin[i] = i * 2;

    indexes[i * 2] = ctx.xind(origen - 1, i);
    indexes[i * 2 + 1] = ctx.xind(destino - 1, i);

    asprintf(&rownames[i], "col_%d_%d_%d", origen, destino, i + 1);
  }

  char *sense = &sentido[0];
  double *rhs = &indep[0];
  int *matbeg = &begin[0];
  int *matind = &indexes[0];
  double *matval = &coefs[0];
  char **rn = &rownames[0];
  int nzcnt = rcnt * 2;

  int status = CPXaddrows(ctx.env, ctx.lp, ccnt, rcnt, nzcnt, rhs, sense,
                          matbeg, matind, matval, NULL, rn);

  if (status) {
    cerr << "Problema agregando las restricciones para la arista (" << origen
         << "," << destino << ")" << endl;
    exit(1);
  }

  for (int i = 0; i < rcnt; i++) {
    free(rownames[i]);
  }
}
void generarLP(lpcontext &ctx, Grafo &grafo) {
  // Generamos las variables
  agregarVariablesParaNodos(ctx);

  agregarVariablesParaColores(ctx);

  // Generamos las restricciones WP
  agregarRestriccionesDeColor(ctx);

  // Generamos las restricciones para las aristas
  for (auto const &a : grafo.getAristas()) {
    agregarRestriccionDeColoreo(ctx, a.getOrigen(), a.getDestino());
  }

  // Generamos las restricciones para las particiones
  int numParticion = 1;
  for (auto const &p : grafo.getParticiones()) {
    agregarRestriccionDeParticion(ctx, p, numParticion++);
  }

  // Escribimos el problema a un archivo .lp.
  int status = CPXwriteprob(ctx.env, ctx.lp, "modelo.lp", NULL);
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
  status = CPXsetdblparam(env, CPX_PARAM_TILIM, 2 * 3600);

  if (status) {
    cerr << "Problema seteando el tiempo limite" << endl;
    exit(1);
  }
}

void setearParametrosDeCPLEXParaBranchAndBoundPuro(CPXENVptr env) {
  // Para que haga Branch & Bound:
  int status =
      CPXsetintparam(env, CPX_PARAM_MIPSEARCH, CPX_MIPSEARCH_TRADITIONAL);
  if (status) {
    cerr << "Problema seteando el parametro CPX_PARAM_MIPSEARCH en "
            "CPX_MIPSEARCH_TRADITIONAL"
         << endl;
    exit(1);
  }

  // Para facilitar la comparación evitamos paralelismo:
  status = CPXsetintparam(env, CPX_PARAM_THREADS, 1);
  if (status) {
    cerr << "Problema seteando el parametro CPX_PARAM_THREADS en 1" << endl;
    exit(1);
  }

  // Para que no se adicionen planos de corte:
  status = CPXsetintparam(env, CPX_PARAM_EACHCUTLIM, 0);
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
  solstat = CPXgetstat(env, lp);
  CPXgetstatstring(env, solstat, statstring);
  string statstr(statstring);
  cout << endl << "Resultado de la optimizacion: " << statstring << endl;
  return endtime - inittime;
}

void generarResultados(CPXENVptr env, CPXLPptr lp, int cantidadDeNodos,
                       double tiempoDeCorridaRecorrerArbol,
                       double tiempoDeCorridaTotal,
                       const string &nombreDelArchivoDeSalida) {
  double objval;
  int status = CPXgetobjval(env, lp, &objval);

  if (status) {
    cerr << "Problema obteniendo valor de mejor solucion." << endl;
    exit(1);
  }

  cout << "Datos de la resolucion: "
       << "\t" << objval << "\t" << tiempoDeCorridaTotal << endl;
  cout << "Tiempo de corrida recorrer el arbol: "
       << tiempoDeCorridaRecorrerArbol << endl;

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
  int longitudNombreVariable = 6 + 3 * longitudNodo;
  int storespace = cantidadDeVariables * longitudNombreVariable;
  char *namestore = new char[storespace];
  char **names = new char *[cantidadDeVariables];
  int sp;
  status = CPXgetcolname(env, lp, names, namestore, storespace, &sp, 0,
                         cantidadDeVariables - 1);

  if (status) {
    cerr << "Problema obteniendo la solucion del LP." << endl;
    exit(1);
  }

  // Solo escribimos las variables distintas de cero (tolerancia, 1E-05).
  solfile << "Tiempo de corrida total: " << tiempoDeCorridaTotal << endl;
  solfile << "Tiempo de corrida recorrer el arbol: "
          << tiempoDeCorridaRecorrerArbol << endl;
  solfile << "Valor de la funcion objetivo: " << objval << endl;
  for (int i = 0; i < cantidadDeVariables; i++) {
    if (sol[i] > TOL) {
      solfile << names[i] << " = " << sol[i] << endl;
    }
  }

  delete[] names;
  delete[] namestore;

  delete[] sol;
  solfile.close();
}

/**
* Lee el archivo de entrada y devuelve el grafo que representa
*/
Grafo leerArchivoDeEntrada(const string &nombreDelArchivo) {
  ifstream archivo;
  int cantidadDeNodos;

  cout << "Nombre del archivo: " << nombreDelArchivo << endl;
  archivo.open(nombreDelArchivo);
  if (archivo.fail()) {
    cout << "No se pudo abrir el archivo: " << nombreDelArchivo << endl;
    exit(1);
  }

  string tipoDeLinea, descarte;
  int origen, destino;

  Grafo grafo;

  while (!archivo.eof()) {
    archivo >> tipoDeLinea;
    if (tipoDeLinea == "p") {
      // Es la linea que describe cantidad de nodos y de aristas
      archivo >> descarte;
      archivo >> cantidadDeNodos;

      grafo.setCantidadDeNodos(cantidadDeNodos);
    }

    else if (tipoDeLinea == "v") {
      // Es una particion
      int cantidadDeNodosEnLaParticion;
      archivo >> cantidadDeNodosEnLaParticion;

      Particion particion;
      int nodo;
      for (int i = 0; i < cantidadDeNodosEnLaParticion; i++) {
        archivo >> nodo;
        particion.agregarNodo(nodo);
      }
      grafo.agregarParticion(particion);
    }

    else if (tipoDeLinea == "e") {
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
  int *indices = new int[cantidadDeVariables];
  char *types = new char[cantidadDeVariables];

  for (int i = 0; i < cantidadDeVariables; i++) {
    indices[i] = i;
    types[i] = type;
  }

  int status = CPXchgctype(env, lp, cantidadDeVariables, indices, types);

  delete[] indices;
  delete[] types;

  if (status) {
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
* Ordena los valores de forma descendiente y los indices los alinea a las
* posiciones de los valores.
* Utilizo selection sort.
*/
void ordenarDescendientemente(double *valores, int *indices, int cantidad) {
  double valorMaximo;
  int indiceMaximo;
  for (int i = 0; i < cantidad; i++) {
    indiceMaximo = 0;
    valorMaximo = 0;
    for (int j = 0; j < cantidad; j++) {
      if (valores[j] > valorMaximo) {
        valorMaximo = valores[j];
        indiceMaximo = j;
      }
    }
    swap(valores[i], valores[indiceMaximo]);
    swap(indices[i], indices[indiceMaximo]);
  }
}

/**
* Agrega una restriccion al lp por la suma de los nodos de color j menor o igual
* al wj
*/
void agregarDesigualdadALP(CPXENVptr env, CPXLPptr lp, list<int> nodos,
                           int colorJ, int cantidadDeNodos, int coeficienteWj,
                           char *nombreDesigualdad) {
  // Estos valores indican:
  // ccnt = numero nuevo de columnas en las restricciones.
  // rcnt = cuantas restricciones se estan agregando.
  // nzcnt = # de coeficientes != 0 a ser agregados a la matriz. Solo se pasan
  // los valores que no son cero.

  int columnasAAgregar = 0;
  int restriccionesAAgregar = 1;
  int coeficientesDistintosDe0 = nodos.size() + 1;

  char *sense = new char[restriccionesAAgregar];   // Sentido de la desigualdad.
                                                   // 'G' es mayor o igual y 'E'
                                                   // para igualdad.
  double *rhs = new double[restriccionesAAgregar]; // Termino independiente de
                                                   // las restricciones.
  int *matbeg = new int[restriccionesAAgregar]; // Posicion en la que comienza
                                                // cada restriccion en matind y
                                                // matval.
  int *matind = new int[restriccionesAAgregar * coeficientesDistintosDe0 +
                        1]; // Array con los indices de las variables con
                            // coeficientes != 0 en la desigualdad.
  double *matval = new double[restriccionesAAgregar * coeficientesDistintosDe0 +
                              1]; // Array que en la posicion i tiene
                                  // coeficiente ( != 0) de la variable
                                  // cutind[i] en la restriccion.
  char *rownames[1];              // es siempre 1 restriccion a agregar.

  sense[0] = 'L'; // Son por menor o igual
  rhs[0] = 0.0;   // El termino independiente siempre es 0
  rownames[0] = nombreDesigualdad;
  matbeg[0] = 0; // ASI ESTABA EN EL EJEMPLO, NO ENTIENDO BIEN QUE ES

  int i = 0;
  list<int>::iterator it;
  for (it = nodos.begin(); it != nodos.end(); ++it) {
    // El nodo i con el color j es sol[i*cantidadDeNodos + j]
    matind[i] = (*it - 1) * cantidadDeNodos +
                (colorJ - 1); // El menos 1 es porque los nodos arrancan de 1 y
                              // las variables de 0
    matval[i] = 1;            // El valor de cada coeficiente de x es 1
    i++;
  }
  // El color j es sol[cantidadDeNodos*cantidadDeNodos + j]
  matind[i] = cantidadDeNodos * cantidadDeNodos + (colorJ - 1);
  matval[i] = coeficienteWj; // El valor del coeficiente de wj

  // Esta rutina agrega la restriccion al lp.
  int status = CPXaddrows(env, lp, columnasAAgregar, restriccionesAAgregar,
                          coeficientesDistintosDe0, rhs, sense, matbeg, matind,
                          matval, NULL, rownames);
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
int buscarCliqueMaximalQueVioleLaDesigualdadCliqueYAgregarla(
    CPXENVptr env, CPXLPptr lp, double *valorVariablePorColor, int *indices,
    int colorJ, double valorWj, Grafo grafo, int numeroDeDesigualdadClique) {
  int cantidadDeNodos = grafo.getCantidadDeNodos();
  int nodo;
  int indicePrimerNodoNoUsado = 0;
  int coeficienteWj = -1;
  list<int> clique;
  while (indicePrimerNodoNoUsado < cantidadDeNodos) {
    clique.clear();

    clique.push_back(indices[indicePrimerNodoNoUsado]);
    double sumaDeLaClique = valorVariablePorColor[0];
    for (int i = indicePrimerNodoNoUsado + 1; i < cantidadDeNodos; i++) {
      nodo = indices[i];
      if (grafo.esAdyacenteATodos(clique, nodo)) {
        clique.push_back(nodo);
        sumaDeLaClique += valorVariablePorColor[i];
      }
    }

    if (sumaDeLaClique > valorWj) {
      // La desigualdad de esta clique viola a la solucion actual
      // Genero nueva restriccion
      int longitudNombreDeDesigualdad =
          2 + ceil(log10(numeroDeDesigualdadClique));
      char *nombreDeLaDesigualdad = new char[longitudNombreDeDesigualdad];
      sprintf(nombreDeLaDesigualdad, "k_%d", numeroDeDesigualdadClique);
      agregarDesigualdadALP(env, lp, clique, colorJ, cantidadDeNodos,
                            coeficienteWj, nombreDeLaDesigualdad);
      numeroDeDesigualdadClique++;
    }

    indicePrimerNodoNoUsado++;
  }

  return numeroDeDesigualdadClique;
}

/**
* Auxiliar sacar a un Util
*/
void desplazarADerechaUnaPosicionAPartirDe(int *items, int desde,
                                           int cantidad) {
  for (int i = cantidad - 1; i >= desde; i--) {
    items[i + 1] = items[i];
  }
}

/**
* Auxiliar sacar a un Util
*/
void desplazarAIzquierdaUnaPosicionAPartirDe(int *items, int desde,
                                             int cantidad) {
  for (int i = desde; i < cantidad; i++) {
    items[i - 1] = items[i];
  }
}
/**
* Auxiliar sacar a un Util
*/
int indiceEn(int *items, int item, int cantidad) {
  for (int i = 0; i < cantidad; i++) {
    if (items[i] == item) {
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
int buscarAgujeroImparQueVioleLaDesigualdadAgujeroImparYAgregarla(
    CPXENVptr env, CPXLPptr lp, double *valorVariablePorColor, int *indices,
    int colorJ, double valorWj, Grafo grafo,
    int numeroDeDesigualdadAgujeroImpar) {

  int cantidadDeNodos = grafo.getCantidadDeNodos();
  int nodo;
  double sumaDelAgujero = 0;
  int *agujero = new int[cantidadDeNodos];
  int cantidadDeNodosEnElAgujero = 0;

  // Comienzo el agujero con una arista que tenga al nodo de mayor valor posible
  int origen = 0;
  int destino;

  // while(origen < cantidadDeNodos-1) {

  cantidadDeNodosEnElAgujero = 0;
  bool encontreArista = false;
  while (origen < cantidadDeNodos - 1 && !encontreArista) {
    nodo = indices[origen];
    destino = origen + 1;
    while (destino < cantidadDeNodos && !encontreArista) {
      if (grafo.sonAdyacentes(nodo, indices[destino])) {
        // Si son adyacentes entonces tengo una arista, agrego los primeros
        // nodos
        agujero[cantidadDeNodosEnElAgujero] = indices[origen];
        sumaDelAgujero += valorVariablePorColor[origen];
        cantidadDeNodosEnElAgujero++;
        agujero[cantidadDeNodosEnElAgujero] = indices[destino];
        sumaDelAgujero += valorVariablePorColor[destino];
        cantidadDeNodosEnElAgujero++;
        encontreArista = true;
      }
      destino++;
    }
    origen++;
  }

  // cout << "El origen quedo: " << origen << ", el destino quedo: " << destino
  // << endl;

  if (!encontreArista) {
    cout << "No encontre arista para comenzar a generar el agujero" << endl;
    return numeroDeDesigualdadAgujeroImpar;
  }
  // Hasta aca solo tenemos dos nodos en el agujero

  // Agrego nodos al agujero buscando entre cada par de nodos si
  // puedo poner un nodo en el medio
  bool agregueNodoAlAgujero;
  int ultimoNodoAgregado;
  double valorUltimoNodoAgregado;
  int i, j;
  list<int> nodos;
  i = 1;

  // while (i < cantidadDeNodosEnElAgujero) {

  j = i;
  agregueNodoAlAgujero = false;
  while (j < cantidadDeNodosEnElAgujero) { // && !agregueNodoAlAgujero) {
    nodos.clear();
    nodos.push_back(agujero[j]);
    nodos.push_back(agujero[j - 1]);

    int k = 0;
    // nodo = 1;
    // while(nodo <= cantidadDeNodos && !agregueNodoAlAgujero) {
    while (k < cantidadDeNodos) { // && !agregueNodoAlAgujero) {
      nodo = indices[k];
      if (grafo.esAdyacenteATodos(nodos, nodo)) {
        // Si el nodo es adyacente a los dos entonces lo puedo meter en el medio
        if (indiceEn(agujero, nodo, cantidadDeNodosEnElAgujero) == -1) {
          // El nodo no pertenece al agujero
          desplazarADerechaUnaPosicionAPartirDe(agujero, j,
                                                cantidadDeNodosEnElAgujero);
          agujero[j] = nodo;
          valorUltimoNodoAgregado = valorVariablePorColor[k];
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

  if (cantidadDeNodosEnElAgujero < 3) {
    cout << "No encontre un agujero" << endl;
    return numeroDeDesigualdadAgujeroImpar;
  }

  if (cantidadDeNodosEnElAgujero % 2 == 0) {
    // Quiere decir que es un agujero par, debo sacar un nodo
    int indiceEnAgujero =
        indiceEn(agujero, ultimoNodoAgregado, cantidadDeNodosEnElAgujero);
    // Saco el ultimo nodo que agregue
    desplazarAIzquierdaUnaPosicionAPartirDe(agujero, indiceEnAgujero + 1,
                                            cantidadDeNodosEnElAgujero);
    cantidadDeNodosEnElAgujero--;
    sumaDelAgujero -= valorUltimoNodoAgregado;
  }

  int coeficienteWj = (cantidadDeNodosEnElAgujero - 1) / 2;
  if (sumaDelAgujero > coeficienteWj * valorWj) {
    // La desigualdad de este agujero impar viola a la solucion actual
    // Genero nueva restriccion
    list<int> agujeroImpar;
    for (int i = 0; i < cantidadDeNodosEnElAgujero; i++) {
      agujeroImpar.push_back(agujero[i]);
    }
    int longitudNombreDeDesigualdad =
        2 + ceil(log10(numeroDeDesigualdadAgujeroImpar));
    char *nombreDeLaDesigualdad = new char[longitudNombreDeDesigualdad];
    sprintf(nombreDeLaDesigualdad, "a_%d", numeroDeDesigualdadAgujeroImpar);

    agregarDesigualdadALP(env, lp, agujeroImpar, colorJ, cantidadDeNodos,
                          -coeficienteWj, nombreDeLaDesigualdad);
    numeroDeDesigualdadAgujeroImpar++;
  }
  //}
  return numeroDeDesigualdadAgujeroImpar;
}

void agregarPlanosDeCorte(CPXENVptr env, CPXLPptr lp, Grafo grafo,
                          int iteracionesPlanosDeCorte) {
  int cantidadDeNodos = grafo.getCantidadDeNodos();
  int cantidadDeVariables = CPXgetnumcols(env, lp);
  double *solucion = new double[cantidadDeVariables];
  double *valorVariablePorColor = new double[cantidadDeNodos];
  int *indices = new int[cantidadDeNodos];
  int numeroDeDesigualdadClique = 1;
  int numeroDeDesigualdadAgujeroImpar = 1;
  bool agregueClique, agregueAgujero;
  double inittime, endtime;

  for (int i = 0; i < iteracionesPlanosDeCorte; i++) {
    // Tomamos el tiempo de resolucion utilizando CPXgettime.
    CPXgettime(env, &inittime);

    resolverLP(env, lp);
    CPXgetx(env, lp, solucion, 0, cantidadDeVariables - 1);

    // Para cada color busco desigualdades que violen la solucion obtenida
    for (int j = 0; j < cantidadDeNodos; j++) {
      // Para cada color buscamos una clique maximal y agregamos la restriccion
      // El nodo i con el color j es sol[i*cantidadDeNodos + j]
      // El color j es sol[cantidadDeNodos*cantidadDeNodos + j]
      for (int k = 0; k < cantidadDeNodos; k++) {
        valorVariablePorColor[k] = solucion[k * cantidadDeNodos + j];
        indices[k] = k + 1;
      }
      double valorWj = solucion[cantidadDeNodos * cantidadDeNodos + j];
      int colorJ = j + 1;
      ordenarDescendientemente(valorVariablePorColor, indices, cantidadDeNodos);
      numeroDeDesigualdadClique =
          buscarCliqueMaximalQueVioleLaDesigualdadCliqueYAgregarla(
              env, lp, valorVariablePorColor, indices, colorJ, valorWj, grafo,
              numeroDeDesigualdadClique);
      numeroDeDesigualdadAgujeroImpar =
          buscarAgujeroImparQueVioleLaDesigualdadAgujeroImparYAgregarla(
              env, lp, valorVariablePorColor, indices, colorJ, valorWj, grafo,
              numeroDeDesigualdadAgujeroImpar);
    }
    // Tomamos el tiempo de resolucion utilizando CPXgettime.
    CPXgettime(env, &endtime);
    cout << "La iteracion " << i + 1
         << " de planos de corte duro: " << endtime - inittime << endl;
  }
  delete[] valorVariablePorColor;
  delete[] indices;
  delete[] solucion;
}

int main(int argc, char **argv) {
  if (argc < 3) {
    cerr << "Uso: " << argv[0]
         << " <archivoInstancia> <archivoSalida> [cantPlanos]" << endl;
    exit(1);
  }
  // Leemos el archivo y obtenemos el grafo
  const string instanceFilename(argv[1]);
  const string outputFilename(argv[2]);

  int iteracionesPlanosDeCorte = 3; // Por default hace 3 iteraciones
  if (argv[3] != NULL) {
    iteracionesPlanosDeCorte = atoi(argv[3]);
  }

  cout << "Se utilizan " << iteracionesPlanosDeCorte
       << " iteraciones de planos de corte" << endl;
  Grafo grafo = leerArchivoDeEntrada(instanceFilename);
  cout << "El grafo tiene " << grafo.getCantidadDeNodos() << " nodos" << endl;
  cout << "El grafo tiene " << grafo.getCantidadDeAristas() << " aristas"
       << endl;
  cout << "El grafo tiene " << grafo.getCantidadDeParticiones()
       << " particiones" << endl;

  // Creamos el LP asociado al problema
  int status;
  CPXENVptr env; // Puntero al entorno.
  CPXLPptr lp;   // Puntero al LP

  // Creo el entorno.
  env = CPXopenCPLEX(&status);
  if (env == NULL) {
    cerr << "Error creando el entorno" << endl;
    exit(1);
  }

  // Creo el LP.
  lp = CPXcreateprob(env, &status, instanceFilename.c_str());
  if (lp == NULL) {
    cerr << "Error creando el LP" << endl;
    exit(1);
  }

  lpcontext ctx = {
    env : env,
    lp : lp,
    nodos : grafo.getCantidadDeNodos(),
    colores : grafo.getCantidadDeParticiones()
  };

  setearParametrosParaCplex(env);
  setearParametrosDeCPLEXParaBranchAndBoundPuro(env);

  // Ahora si generamos el LP a partir del grafo
  generarLP(ctx, grafo);
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

  double tiempoDeCorridaRecorrerArbol = resolverLP(env, lp);

  // Tomamos el tiempo de resolucion utilizando CPXgettime.
  status = CPXgettime(env, &endtime);

  double tiempoDeCorridaTotal = endtime - inittime;
  cout << "El tiempo total de corrida fue: " << tiempoDeCorridaTotal << endl;

  generarResultados(env, lp, grafo.getCantidadDeNodos(),
                    tiempoDeCorridaRecorrerArbol, tiempoDeCorridaTotal,
                    outputFilename);

  return 0;
}
