#include <ilcplex/ilocplex.h>
#include <ilcplex/cplex.h>
ILOSTLBEGIN
#include <string>
#include <vector>
#include <list>
#include <set>
#include <algorithm>
#include "Grafo.h"

#define TOL 1E-05
#define IT_PLANOS_DE_CORTE 3

typedef struct t_ctx {
  CPXENVptr env;
  CPXLPptr lp;

  int nodos;
  int colores;

  int cliques;
  int agujeros;

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
    char **cnames = &colnames[0];

    int status = CPXnewcols(ctx.env, ctx.lp, nodos, obj, lb, ub, NULL, cnames);
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
  char **cnames = &colnames[0];

  int status = CPXnewcols(ctx.env, ctx.lp, colores, obj, lb, ub, NULL, cnames);
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
  status = CPXsetintparam(env, CPX_PARAM_SCRIND, CPX_OFF);

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
  
  status = CPXsetintparam(env, CPX_PARAM_LANDPCUTS, -1);
  
  status = CPXsetintparam(env, CPX_PARAM_ADVIND, 0);
  if (status) {
    cerr << "Problema seteando el parametro CPX_ADVINV en 0" << endl;
    exit(1);
  }
  
  status = CPXsetintparam(env, CPX_PARAM_PRESLVND, -1);
  status = CPXsetintparam(env, CPX_PARAM_REPEATPRESOLVE, 0);
  status = CPXsetintparam(env, CPX_PARAM_RELAXPREIND, 0);
  status = CPXsetintparam(env, CPX_PARAM_REDUCE, 0);
}

double resolverLPGenerico(lpcontext &ctx, bool esEntera) {
  // Resolvemos usando CPLEX pero branch and bound
  double inittime, endtime;
  int status;

  // Tomamos el tiempo de resolucion utilizando CPXgettime.
  status = CPXgettime(ctx.env, &inittime);

  // Optimizamos el problema.
  if (esEntera) {
    cerr << "def" << endl;
    status = CPXmipopt(ctx.env, ctx.lp);
  } else {
    cerr << "asd" << endl;
    status = CPXlpopt(ctx.env, ctx.lp);
  }

  if (status) {
    cerr << "Problema optimizando CPLEX, status: " << status << endl;
    exit(1);
  }

  status = CPXgettime(ctx.env, &endtime);

  if (status) {
    cerr << "Problema obteniendo el tiempo luego de terminar el lp" << endl;
    exit(1);
  }

  // Chequeamos el estado de la solucion.
  int solstat;
  char statstring[510];
  solstat = CPXgetstat(ctx.env, ctx.lp);
  CPXgetstatstring(ctx.env, solstat, statstring);
  string statstr(statstring);
  cout << endl << "Resultado de la optimizacion: " << statstring << endl;
  return endtime - inittime;
}

double resolverLP(lpcontext &ctx) {
  return resolverLPGenerico(ctx, true);
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

/*
   convertirVariablesLP convierte todas las variables del LP al tipo dado
   Esta funcion es util para relajar un problema de programacion lineal,
   cambiando variables binarias por continuas, y viceversa.
*/
void convertirVariablesLP(lpcontext &ctx, char type) {
  int cantidadDeVariables = CPXgetnumcols(ctx.env, ctx.lp);

  vector<char> types(cantidadDeVariables, type);
  vector<int> indices(cantidadDeVariables, 0);

  for (int i = 0; i < cantidadDeVariables; i += 1) {
    indices[i] = i;
  }

  int status =
      CPXchgctype(ctx.env, ctx.lp, cantidadDeVariables, &indices[0], &types[0]);

  if (status) {
    cerr << "Error cambiando los tipos de las variables del problema" << endl;
    exit(1);
  }
}

void restringirLP(lpcontext &ctx) { convertirVariablesLP(ctx, CPX_BINARY); }

/**
* Ordena los valores de forma descendiente y los indices los alinea a las
* posiciones de los valores.
* Por ejemplo, a <4, 1, 6> <0, 1, 2> los convierte en <6, 4, 1>, <2, 0, 1>
* Utilizo selection sort.
*/
void ordenarDescendentemente(vector<double> &valores, vector<int> &indices) {
  double valorMaximo;
  unsigned int indiceMaximo;

  for (unsigned int i = 0; i < valores.size(); i += 1) {
    indiceMaximo = i;
    valorMaximo = valores[i];

    for (unsigned int j = i + 1; j < valores.size(); j += 1) {
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
* Dada una lista de nodos (a1, ..., ak), un color j con un coeficiente, agrega
* la restriccion:
* xa1_j + xa2_j + ... + xak_j + cj * wj <= 0.0
*/
void agregarDesigualdadALP(lpcontext &ctx, set<int> nodos, int color,
                           int coefWj, char *nombreDesigualdad) {
  int ccnt = 0;
  int rcnt = 1;
  int nzcnt = nodos.size() + 1;
  char sentido = 'L';
  double indep = 0.0;
  int begin = 0;
  vector<int> indices;
  vector<double> coefs;

  // El nodo i con el color j es x_n_j
  for (const auto &n : nodos) {
    indices.push_back(ctx.xind(n, color));
    coefs.push_back(1.0);
  }

  cerr << "Esta desigualdad consiste en:" << endl;
  int vars = CPXgetnumcols(ctx.env, ctx.lp);
  int espacio = 20000;
  int surplus = 0;
  vector<char *> nombres(vars, NULL);

  char *store = (char *)malloc(espacio);

  CPXgetcolname(ctx.env, ctx.lp, &nombres[0], store, espacio, &surplus, 0,
                vars - 1);

  for (unsigned int i = 0; i < indices.size(); i++) {
    cerr << nombres[indices[i]] << " + ";
  }

  // ultima variable: coeficienteWj * wj
  indices.push_back(ctx.wind(color));
  coefs.push_back(coefWj);

  cerr << coefs[coefs.size() - 1] << nombres[indices[indices.size() - 1]]
       << endl;
  free(store);

  int status =
      CPXaddrows(ctx.env, ctx.lp, ccnt, rcnt, nzcnt, &indep, &sentido, &begin,
                 &indices[0], &coefs[0], NULL, &nombreDesigualdad);
  if (status) {
    cerr << "Problema agregando restriccion por desigualdad clique" << endl;
    exit(1);
  }
}

/**
* Dada la solucion para un color ordenada descendentemente busca una clique
* cuya desigualdad sea violada por la solucion y se agrega la restriccion
* correspondiente al LP
*/
void agregarCliquesQueViolenDesigualdad(lpcontext &ctx,
                                        const vector<double> &valores,
                                        const vector<int> &indices, int color,
                                        double valorWj, const Grafo &grafo) {

  int nodos = ctx.nodos;
  int coeficienteWj = -1;

  int numClique = ctx.cliques;

  /*
    Este algoritmo funciona de la siguiente manera:
      Recorre la lista de nodos no usados en orden (del de mayor valor al de
      menor valor), y se fija si se puede armar una clique que viole
      la desigualdad.
  */

  set<set<int>> cliquesUsadas;
  for (int i = 0; i < nodos; i++) {
    set<int> clique;

    clique.insert(indices[i]);
    double sumaClique = valores[i];
    for (int j = i + 1; j < nodos; j++) {
      int nodo = indices[j];
      if (grafo.esAdyacenteATodos(clique, nodos)) {
        clique.insert(nodo);
        sumaClique += valores[j];
      }
    }

    // solo agregamos cliques que usen al menos el 10% de los nodos
    if (clique.size() < (unsigned int) ctx.nodos / 10) {
      continue;
    }

    bool estaRepetido = false;
    for (const auto &c : cliquesUsadas) {
      if (includes(c.begin(), c.end(), clique.begin(), clique.end())) {
        estaRepetido = true;
        break;
      }
    }
    if (estaRepetido)
      continue;

    if (sumaClique > valorWj) {
      char *desigualdad = NULL;
      asprintf(&desigualdad, "k_%d", numClique);
      // cerr << "Se agrega clique " << numClique << " color " << color;
      // cerr << " nodos " << clique.size() << " " << desigualdad;
      agregarDesigualdadALP(ctx, clique, color, coeficienteWj, desigualdad);
      free(desigualdad);
      numClique += 1;
      cliquesUsadas.insert(clique);
    }
  }

  ctx.cliques = numClique;
}

void agregarPlanoDeCorte(lpcontext &ctx, Grafo &grafo) {
  int nodos = ctx.nodos;
  int colores = ctx.colores;
  int vars = CPXgetnumcols(ctx.env, ctx.lp);

  vector<double> solucion(vars, 0.0);

  // resolvemos el LP Relajado.
  double inittime, endtime;
  CPXgettime(ctx.env, &inittime);
  resolverLPGenerico(ctx, true);
  CPXgettime(ctx.env, &endtime);
  cout << "ResolverLP Tardo: " << endtime - inittime << endl;
  CPXgetx(ctx.env, ctx.lp, &solucion[0], 0, vars - 1);
  CPXgettime(ctx.env, &inittime);
  // Para cada color busco desigualdades que violen la solucion obtenida
  for (int j = 0; j < colores; j++) {
    // Para cada color buscamos una clique maximal y agregamos la restriccion
    // La suma de los nodos pintados con ese color tiene que ser que el wj
    vector<double> valores(nodos, 0.0); // xij
    // indices tiene el indice de la variable (la iesima variable)
    vector<int> indices(nodos, 0);

    for (int k = 0; k < nodos; k++) {
      valores[k] = solucion[ctx.xind(k, j)];
      indices[k] = k;
    }

    double valorWj = solucion[ctx.wind(j)];
    int color = j;

    ordenarDescendentemente(valores, indices);

    agregarCliquesQueViolenDesigualdad(ctx, valores, indices, color, valorWj,
                                       grafo);
  }
  CPXgettime(ctx.env, &endtime);
  cout << "Agregar las restricciones tardo: " << endtime - inittime << endl;
}

/*
  agregarPlanosDeCorte busca desigualdades de ciertas familias:
    - Agujero Impar
    - Clique Maxima
  y trata de agregar las restricciones violadas al LP para achicar
  el espacio de busqueda.

  La forma en la que se buscan las desigualdades es la siguiente:

*/
void agregarPlanosDeCorte(lpcontext &ctx, Grafo grafo,
                          int iteracionesPlanosDeCorte) {

  for (int i = 0; i < iteracionesPlanosDeCorte; i++) {
    double inittime, endtime;
    CPXgettime(ctx.env, &inittime);
    agregarPlanoDeCorte(ctx, grafo);
    // Tomamos el tiempo de resolucion utilizando CPXgettime.
    CPXgettime(ctx.env, &endtime);
    cout << "Iteracion " << i + 1 << " de planos de corte duro: ";
    cout << endtime - inittime << endl;
  }
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
  CPXFILEptr logfile = CPXfopen("/tmp/cplex.log", "a");
  if (logfile == NULL) {
    cerr << "Problema abriendo el archivo log" << endl;
    exit(1);
  }
  
  status = CPXsetlogfile(env, logfile);
  if (status) {
    cerr << "Problema seteando el archivo log" << endl;
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
    colores : grafo.getCantidadDeParticiones(),
    cliques : 0,
    agujeros : 0
  };

  setearParametrosParaCplex(env);
  setearParametrosDeCPLEXParaBranchAndBoundPuro(env);

  // Ahora si generamos el LP a partir del grafo
  generarLP(ctx, grafo);
  double inittime, endtime;

  // Tomamos el tiempo de resolucion utilizando CPXgettime.
  status = CPXgettime(env, &inittime);
  convertirVariablesLP(ctx, CPX_CONTINUOUS);
  // Agregamos los planos de corte
  agregarPlanosDeCorte(ctx, grafo, iteracionesPlanosDeCorte);
  restringirLP(ctx);

  // Escribimos el LP
  CPXwriteprob(env, lp, "modelo.lp", NULL);

  double tiempoDeCorridaRecorrerArbol = resolverLP(ctx);

  // Tomamos el tiempo de resolucion utilizando CPXgettime.
  status = CPXgettime(env, &endtime);

  double tiempoDeCorridaTotal = endtime - inittime;
  cout << "El tiempo total de corrida fue: " << tiempoDeCorridaTotal << endl;

  generarResultados(env, lp, grafo.getCantidadDeNodos(),
                    tiempoDeCorridaRecorrerArbol, tiempoDeCorridaTotal,
                    outputFilename);

  status = CPXfclose(logfile);
  if (status) {
    cerr << "Problema cerrando el archivo de logs" << endl;
    exit(1);
  }

  return 0;
}
