#include <map>
#include <string>
#include <iostream>
#include <fstream>
#include <cmath>
#include <mpi.h>


using namespace std;


//carga el vocabulario en un array de strings
//para simplificar el código el vocabulario y su size se obtienen del jupyter notebook
void load_vocabulary(string file_name, string* vocabulario, long long int vocabulario_size) {
    ifstream in("O:/itam/ComParalelo/Parallel-bag-of-words/data/"+file_name);
    if (!in) {
        cerr << "Couldn't read file: " << file_name << "\n";
    }
    long long int word_number = 0;
    while (!in.eof() && (word_number < vocabulario_size)) {
        //lee in hasta encontrar un salto de linea y lo va guardado en vocabulario
        getline(in, vocabulario[word_number], '\n');
        word_number++;
    }
}

//cuenta las palabras de varios archivos y las guarda en sus diccionarios
void count_parallel_words(string* libros, int libros_size, string* vocabulario,
                        long long int vocabulario_size, map<string, int>* diccionarios,
                        int** matriz, int* aux,
                        int process_id, int num_processes) {
    
    MPI_Comm_size(MPI_COMM_WORLD, &num_processes);
	MPI_Comm_rank(MPI_COMM_WORLD, &process_id);


    //validamos la cantidad de libros
    if(libros_size != num_processes){
        cerr << "El numero de procesos debe ser igual al numero de libros\n";
    }

    cout << "Soy: " << process_id << "\n";
    cout << "Libro: " << libros[process_id] << "\n";

    //distriubimos los libros
    ifstream in("O:/itam/ComParalelo/Parallel-bag-of-words/data/"+libros[process_id]+".txt");
    if (!in) {
        cerr << "Couldn't read file: " << libros[process_id] << "\n";
    }
    while (!in.eof()) {
        string word;
        //lee coma por coma y lo guarda en word
        getline(in, word, ',');
        //si la palabra esta en el vocabulario
        if (diccionarios[process_id].count(word) > 0) {
            //le suma 1 a su contador
            diccionarios[process_id][word] = diccionarios[process_id][word] + 1;
        }
    }

   
    for (long long int j = 0; j < vocabulario_size; j++) {
        //va guardanddo los valores de cada diccionario en el array auxiliar
        aux[j] = diccionarios[process_id][vocabulario[j]];
    }

    //Juntamos los resultados de cada proceso
    MPI_Gather(&aux, 
            vocabulario_size, //cada array tiene el tamaño del vocabulario
            MPI_INT, //manda la cuenta de cada palabra
            matriz, //matriz donde se guardan los resultados
            vocabulario_size, //enviamos lo mismo que recibimos
            MPI_INT, //x2
            0, 
            MPI_COMM_WORLD);

   
    
}

//guarda la matriz en csv
void matrix_to_CSV(string file_name, map<string, int>* diccionarios, int libros_size, string* vocabulario, long long int vocabulario_size) {
    fstream fout;
    fout.open(file_name, ios::out);
    //recorre los libros
    for (long long int i = 0; i < libros_size; i++) {
        //recorre el vocabulario
        for (long long int j = 0; j < vocabulario_size; j++) {
            //va guardanddo el contador de la palabra j en el libro i
            fout << diccionarios[i][vocabulario[j]] << ",";
        }
        //salto de linea para empezar con otro libro
        fout << "\n";
    }
}

//guarda el dataframe en csv (este incluye el titulo de cada libro y el vocabulario)
//los autores estan consientes en c++ no hay dataframes pero shhhhh
void dataframe_to_CSV(string file_name, map<string, int>* diccionarios, int libros_size, string* vocabulario, long long int vocabulario_size, string* libros) {
    fstream fout;
    fout.open(file_name, ios::out);
    //titulo de la primer columna con libros
    fout << "Libro,";
    for (long long int i = 0; i < vocabulario_size; i++) {
        //titulo de cada columna con vocabulario
        fout << vocabulario[i] << ",";
    }
    //empezar con los libros
    fout << "\n";
    for (long long int i = 0; i < libros_size; i++) {
        //titulo i 
        fout << libros[i] << ",";
        //valores del vocabulario del libro i
        for (long long int j = 0; j < vocabulario_size; j++) {
            fout << diccionarios[i][vocabulario[j]] << ",";
        }
        fout << "\n";
    }
}

int main(int argc, char ** argv){
    
    cout << "Hola mundo\n";
    //numero de libros
    const int libros_size = 6;
    //array de strings con los nombres de los libros
    string libros[libros_size] = {"shakespeare_the_merchant_of_venice", "shakespeare_romeo_juliet", "shakespeare_hamlet",
                         "dickens_a_christmas_carol", "dickens_oliver_twist", "dickens_a_tale_of_two_cities"};
    //tamano del vocabulario
    const long long int vocabulario_size = atoi(argv[1]);
    //array de strings con el vocabulario
    string* vocabulario = new string[vocabulario_size];
    //diccionarios de cada libro
    map<string, int> diccionarios[libros_size];

    //matriz donde se guardan los resultados
    int** matriz = new int*[libros_size];

    cout << "Leyendo vocabulario\n";

    //carga el vocabulario en el array de strings
    load_vocabulary("vocabulario.txt", vocabulario, vocabulario_size);

    cout << "Iniciando diccionarios\n";

    //inicializa los diccionarios con el vocabulario
    for (long long int i = 0; i < libros_size; i++) {
        for (long long int j = 0; j < vocabulario_size; j++) {
            //recorremos el vocabulario y lo guardamos en el diccionario
            diccionarios[i].insert({vocabulario[j], 0});
        }
    }

    cout << "Iniciando matriz\n";

    int num_processes = 0;

	int process_id = 0;  


    MPI_Init(&argc, &argv);
	
    //array auxiliar donde se guardan los resultados de cada proceso
    int* aux = new int[vocabulario_size];

    //contamos las palabras y guardamos los resultados en la matriz
    count_parallel_words(libros, libros_size, vocabulario,
                         vocabulario_size, diccionarios, matriz, 
                         aux, process_id, num_processes);

    MPI_Finalize();

    cout << "Fin imprimiendo\n";

    //imprime los 10 primeros de matriz
    for (long long int i = 0; i < 6; i++) {
        for (long long int j = 0; j < 10; j++) {
            cout << matriz[i][j] << ",";
        }
        cout << "\n";
    }


    //guarda la matriz en csv
    // matrix_to_CSV("bow-p-matrix.csv", diccionarios, libros_size, vocabulario, vocabulario_size);

    //guarda el dataframe en csv
    // dataframe_to_CSV("bow-p-df.csv", diccionarios, libros_size, vocabulario, vocabulario_size, libros);

    return 420;

}