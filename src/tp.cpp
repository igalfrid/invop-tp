#include <ilcplex/ilocplex.h>
#include <ilcplex/cplex.h>
ILOSTLBEGIN
#include <string>
#include <vector>

#define TOL 1E-05

int main(int argc, char **argv) {
  //Leemos el archivo
  ifstream archivo;
  int cantidadDeNodos;
  int cantidadDeAristas;
  int cantidadDeParticiones;
  int numeroDeParticion = 0;

  int num = 0; // num must start at 0
  char * nombreDelArchivo = argv[1];
  cout << "Nombre del archivo: " << nombreDelArchivo << endl;
  archivo.open(nombreDelArchivo);// file containing numbers in 3 columns
  if(archivo.fail()) // checks to see if file opended
  {
    cout << "No se pudo abrir el archivo: " << nombreDelArchivo << endl;
    return 1; // no point continuing if the file didn't open...
  }

  // Genero el problema de cplex.
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
      // Genero las columnas
      // Agrego las columnas por todos cada combinacion de nodo
      double ub[cantidadDeNodos][cantidadDeNodos], lb[cantidadDeNodos][cantidadDeNodos], objfun[cantidadDeNodos][cantidadDeNodos];
      char *colnames[cantidadDeNodos][cantidadDeNodos], xctype[cantidadDeNodos][cantidadDeNodos];
      for(int i = 0; i < cantidadDeNodos; i++) {
        for(int j = 0; j < cantidadDeNodos; j++) {
          ub[i][j] = 1;
          lb[i][j] = 0.0;
          colnames[i][j] = new char[10];
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

      // Agrego las columnas por cada color
      double ubw[cantidadDeNodos], lbw[cantidadDeNodos], objfunw[cantidadDeNodos];
      char *colnamesw[cantidadDeNodos], xctypew[cantidadDeNodos];
      for(int i = 0; i < cantidadDeNodos; i++) {
        ubw[i] = 1;
        lbw[i] = 0.0;
        colnamesw[i] = new char[10];
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

      //Agrego las restricciones wp que me dicen que si una variable p esta seteada, entonces la w tiene que estar seteada
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

          rownames[j] = new char[10];
          sprintf(rownames[j],"wp_%d_%d", i+1, j+1);

          nzcnt+=2;
        }
        // Esta rutina agrega la restriccion al lp.
        status = CPXaddrows(env, lp, ccnt, rcnt, nzcnt, rhs, sense, matbeg, matind, matval, NULL, rownames);
      }
    }

    else if(tipoDeLinea == "v") {
      // Es una particion
      numeroDeParticion++;
      cout << "Particion: ";
      int cantidadDeNodosEnLaParticion;
      int nodo;
      archivo >> cantidadDeNodosEnLaParticion;

      // Agregamos la restriccion
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
      rownames[0] = new char[10];
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
      status = CPXaddrows(env, lp, ccnt, rcnt, nzcnt, rhs, sense, matbeg, matind, matval, NULL, rownames);

      delete[] rhs;
      delete[] matbeg;
      delete[] matind;
      delete[] matval;
      for(int i = 0; i < rcnt; i++) {
        delete[] rownames[i];
      }
    }

    else if(tipoDeLinea == "e" && cantidadDeAristas > 0) {
      // Es una arista
      cantidadDeAristas--;
      archivo >> origen;
      archivo >> destino;
      cout << "Arista (" << origen << "," << destino  << ")" << endl;

      // Genero las restricciones col que me dicen que los nodos adyacentes no pueden tener el mismo color


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

      for(int i = 0; i < rcnt; i++) {
        sense[i] = 'L'; // Son por menor o igual
        rhs[i] = 1.0; // El termino independiente siempre es 1

        matbeg[i] = nzcnt; // ASI ESTABA EN EL EJEMPLO, NO ENTIENDO BIEN QUE ES
        matval[nzcnt] = 1; // El valor del coeficiente de cada termino es 1
        matval[nzcnt + 1] = 1; // El valor del coeficiente de cada termino es 1

        matind[nzcnt] = (origen-1) * cantidadDeNodos + i;
        matind[nzcnt + 1] = (destino-1) * cantidadDeNodos + i;

        rownames[i] = new char[10];
        sprintf(rownames[i],"col_%d_%d_%d", origen, destino, i+1);

        nzcnt+=2;
      }




      // Esta rutina agrega la restriccion al lp.
      status = CPXaddrows(env, lp, ccnt, rcnt, nzcnt, rhs, sense, matbeg, matind, matval, NULL, rownames);

      delete[] rhs;
      delete[] matbeg;
      delete[] matind;
      delete[] matval;
      for(int i = 0; i < rcnt; i++) {
        delete[] rownames[i];
      }


      status = CPXwriteprob(env, lp, "tp.lp", NULL);

    }
  }
  archivo.close();

   // Escribimos el problema a un archivo .lp.
  status = CPXwriteprob(env, lp, "tp.lp", NULL);

  if (status) {
    cerr << "Problema escribiendo modelo" << endl;
    exit(1);
  }

  return 0; // everything went right.
  /*


  // Datos del nodo
  int cantidadDeNodos = 5;
  int cantidadDeAristas = 6;
  int aristas[6][2] = {
    {1,5},
    {2,4},
    {1,2},
    {4,5},
    {3,4},
    {2,5}
  };




  // Datos de la instancia de dieta
  int n = 3;
  double costo[] = {1.8, 2.3,1.5};
  double calorias[] = {170,50,300};
  double calcio[] = {3,400,40};
  double minCalorias = 2000;
  double maxCalorias = 2300;
  double minCalcio = 1200;
  double maxPan = 3;
  double minLeche = 2;

  // Genero el problema de cplex.
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
  lp = CPXcreateprob(env, &status, "instancia de TP");


  if (lp == NULL) {
    cerr << "Error creando el LP" << endl;
    exit(1);
  }


  // Agrego las columnas.
  int cantidadDeVariables = cantidadDeNodos*cantidadDeNodos + cantidadDeNodos;
  status = CPXnewcols(env, lp, cantidadDeVariables, objfun, lb, ub, NULL, colnames);

  if (status) {
    cerr << "Problema agregando las variables CPXnewcols" << endl;
    exit(1);
  }
  */
/*
  // Definimos las variables. No es obligatorio pasar los nombres de las variables, pero facilita el debug. La info es la siguiente:
  double *ub, *lb, *objfun; // Cota superior, cota inferior, coeficiente de la funcion objetivo.
  char *xctype, **colnames; // tipo de la variable (por ahora son siempre continuas), string con el nombre de la variable.
  ub = new double[n];
  lb = new double[n];
  objfun = new double[n];
  xctype = new char[n];
  colnames = new char*[n];

  for (int i = 0; i < n; i++) {
    ub[i] = CPX_INFBOUND;
    lb[i] = 0.0;
    objfun[i] = costo[i];
    xctype[i] = 'C'; // 'C' es continua, 'B' binaria, 'I' Entera. Para LP (no enteros), este parametro tiene que pasarse como NULL. No lo vamos a usar por ahora.
    colnames[i] = new char[10];
  }

  // Nombre de la variable x_m
  sprintf(colnames[0],"x_m");

  // Nombre de la variable x_l y cota inferior
  lb[1] = 2;
  sprintf(colnames[1],"x_l");

  // Nombre de la variable x_p y cota superior
  ub[2] = 3;
  sprintf(colnames[2],"x_p");

  // Agrego las columnas.
  status = CPXnewcols(env, lp, n, objfun, lb, ub, NULL, colnames);

  if (status) {
    cerr << "Problema agregando las variables CPXnewcols" << endl;
    exit(1);
  }

  // Libero las estructuras.
  for (int i = 0; i < n; i++) {
    delete[] colnames[i];
  }

  delete[] ub;
  delete[] lb;
  delete[] objfun;
  delete[] xctype;
  delete[] colnames;


  // CPLEX por defecto minimiza. Le cambiamos el sentido a la funcion objetivo si se quiere maximizar.
  // CPXchgobjsen(env, lp, CPX_MAX);

  // Generamos de a una las restricciones.
  // Estos valores indican:
  // ccnt = numero nuevo de columnas en las restricciones.
  // rcnt = cuantas restricciones se estan agregando.
  // nzcnt = # de coeficientes != 0 a ser agregados a la matriz. Solo se pasan los valores que no son cero.

  int ccnt = 0, rcnt = 3, nzcnt = 0;

  char sense[] = {'G','L','G'}; // Sentido de la desigualdad. 'G' es mayor o igual y 'E' para igualdad.

  double *rhs = new double[rcnt]; // Termino independiente de las restricciones.
  int *matbeg = new int[rcnt]; //Posicion en la que comienza cada restriccion en matind y matval.
  int *matind = new int[3*n]; // Array con los indices de las variables con coeficientes != 0 en la desigualdad.
  double *matval = new double[3*n]; // Array que en la posicion i tiene coeficiente ( != 0) de la variable cutind[i] en la restriccion.

  // Podria ser que algun coeficiente sea cero. Pero a los sumo vamos a tener 3*n coeficientes. CPLEX va a leer hasta la cantidad
  // nzcnt que le pasemos.


  //Restriccion de minimas calorias
  matbeg[0] = nzcnt;
  rhs[0] = minCalorias;
  for (int i = 0; i < n; i++) {
     matind[nzcnt] = i;
     matval[nzcnt] = calorias[i];
     nzcnt++;
  }

  //Restriccion de maximas calorias
  matbeg[1] = nzcnt;
  rhs[1] = maxCalorias;
  for (int i = 0; i < n; i++) {
     matind[nzcnt] = i;
     matval[nzcnt] = calorias[i];
     nzcnt++;
  }

  //Restriccion de minimo calcio
  matbeg[2] = nzcnt;
  rhs[2] = minCalcio;
  for (int i = 0; i < n; i++) {
     matind[nzcnt] = i;
     matval[nzcnt] = calcio[i];
     nzcnt++;
  }

  // Esta rutina agrega la restriccion al lp.
  status = CPXaddrows(env, lp, ccnt, rcnt, nzcnt, rhs, sense, matbeg, matind, matval, NULL, NULL);

  if (status) {
    cerr << "Problema agregando restricciones." << endl;
    exit(1);
  }

  delete[] rhs;
  delete[] matbeg;
  delete[] matind;
  delete[] matval;

  // Seteo de algunos parametros.
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

  // Escribimos el problema a un archivo .lp.
  status = CPXwriteprob(env, lp, "dieta2.lp", NULL);

  if (status) {
    cerr << "Problema escribiendo modelo" << endl;
    exit(1);
  }

  // Tomamos el tiempo de resolucion utilizando CPXgettime.
  double inittime, endtime;
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
     exit(1);
  }

  double objval;
  status = CPXgetobjval(env, lp, &objval);

  if (status) {
    cerr << "Problema obteniendo valor de mejor solucion." << endl;
    exit(1);
  }

  cout << "Datos de la resolucion: " << "\t" << objval << "\t" << (endtime - inittime) << endl;

  // Tomamos los valores de la solucion y los escribimos a un archivo.
  std::string outputfile = "dieta.sol";
  ofstream solfile(outputfile.c_str());


  // Tomamos los valores de todas las variables. Estan numeradas de 0 a n-1.
  double *sol = new double[n];
  status = CPXgetx(env, lp, sol, 0, n - 1);

  if (status) {
    cerr << "Problema obteniendo la solucion del LP." << endl;
    exit(1);
  }


  // Solo escribimos las variables distintas de cero (tolerancia, 1E-05).
  solfile << "Status de la solucion: " << statstr << endl;
  for (int i = 0; i < n; i++) {
    if (sol[i] > TOL) {
      solfile << "x_" << i << " = " << sol[i] << endl;
    }
  }


  delete [] sol;
  solfile.close();

*/
  return 0;
}
