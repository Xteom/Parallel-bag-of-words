#include <map>
#include <string>
#include <iostream>
#include <fstream>
#include <cmath>
#include <mpi.h>


using namespace std;


//carga el vocabulario en un array de strings
//para simplificar el código el vocabulario y su size se obtienen del jupyter notebook
void load_vocabulary(string file_name, string* vocabulario, long long int vocabulario_size,
                    string path = "O:/itam/ComParalelo/Parallel-bag-of-words/" ) {
    ifstream in(path+"data/"+file_name);
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
                        int* matriz_aplanada, int* aux,
                        int process_id, int num_processes, 
                        string path = "O:/itam/ComParalelo/Parallel-bag-of-words/" ) {
    
    //validamos la cantidad de libros
    if(libros_size != num_processes){
        cerr << "El numero de procesos debe ser igual al numero de libros\n";
    } else{
        //distriubimos los libros
        ifstream book(path+"data/"+libros[process_id]+".txt");
        if (!book) {
            cerr << "Couldn't read file: " << libros[process_id] << "\n";
        }
        while (!book.eof()) {
            string word;
            //lee coma por coma y lo guarda en word
            getline(book, word, ',');
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
        MPI_Gather(aux, 
            vocabulario_size, //cada array tiene el tamaño del vocabulario
            MPI_INT, //manda la cuenta de cada palabra
            matriz_aplanada, //matriz donde se guardan los resultados
            vocabulario_size, //enviamos lo mismo que recibimos
            MPI_INT, //x2
            0, //donde lo recibimos 
            MPI_COMM_WORLD);

    }
}

//guarda la matriz_aplanada en csv
void matrix_to_CSV(string file_name, int* matriz_aplanada, int libros_size, long long int vocabulario_size,
                    string path = "O:/itam/ComParalelo/Parallel-bag-of-words/" ) {                        
    fstream fout;
    fout.open(path+"res/"+file_name, ios::out);
    //recorre matriz aplanada y va separando por libros con ayuda de vocabulario_size
    for (int i = 0; i < libros_size; i++) {
        for (long long int j = 0; j < vocabulario_size; j++) {
            fout << matriz_aplanada[i*vocabulario_size+j] << ",";
        }
        fout << "\n";
    }
}



//guarda el dataframe en csv (este incluye el titulo de cada libro y el vocabulario)
//los autores estan consientes en c++ no hay dataframes pero shhhhh
void dataframe_to_CSV(string file_name, string* libros, int libros_size, string* vocabulario,
                        long long int vocabulario_size, int* matriz_aplanada,
                        string path = "O:/itam/ComParalelo/Parallel-bag-of-words/" ) {
    fstream fout;
    fout.open(path+"res/"+file_name, ios::out);
    //titulo de la primer columna con libros
    fout << "Libro,";
    //imprime el vocabulario
    for (long long int j = 0; j < vocabulario_size; j++) {
        fout << vocabulario[j] << ",";
    }
    fout << "\n";
    //recorre matriz aplanada y va separando por libros con ayuda de vocabulario_size
    for (int i = 0; i < libros_size; i++) {
        fout << libros[i] << ",";
        for (long long int j = 0; j < vocabulario_size; j++) {
            fout << matriz_aplanada[i*vocabulario_size+j] << ",";
        }
        fout << "\n";
    }
}

int main(int argc, char ** argv){
    
    //path de data (tiene que ser de raiz) descomentar si se quiere usar
    string path = "O:/itam/ComParalelo/Parallel-bag-of-words/";
    
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
    //aplanada porque na mas no pudimos con matriz normal 
    int matriz_aplanada[libros_size*vocabulario_size]{0};
    //int* matriz[libros_size][vocabulario_size]{0};

    //array auxiliar donde se guardan los resultados de cada proceso
    int aux[vocabulario_size]{0};

    //carga el vocabulario en el array de strings
    load_vocabulary("vocabulario.txt", vocabulario, vocabulario_size);


    int num_processes;
	int process_id = 0;  

    double start;
    double end; 
    double duration;

    //inicializa los diccionarios con el vocabulario
    for (long long int i = 0; i < libros_size; i++) {
        for (long long int j = 0; j < vocabulario_size; j++) {
            //recorremos el vocabulario y lo guardamos en el diccionario
            diccionarios[i].insert({vocabulario[j], 0});
        }
    }

    MPI_Init(&argc, &argv);

    MPI_Comm_size(MPI_COMM_WORLD, &num_processes);
	MPI_Comm_rank(MPI_COMM_WORLD, &process_id);
	

    //marca el tiempo de inicio
    if (process_id == 0) start = MPI_Wtime();

    //contamos las palabras y guardamos los resultados en la matriz
    count_parallel_words(libros, libros_size, vocabulario,
                         vocabulario_size, diccionarios, matriz_aplanada, 
                         aux, process_id, num_processes);
    
    //marca el tiempo de fin
    if (process_id == 0) end = MPI_Wtime();

    MPI_Finalize();


    if (process_id == 0) {
        //calcula el tiempo de ejecucion
        duration = (end - start)*1000000; //se multiplica por 1000000 para obtener el tiempo en microsegundos

        cout << "Guardando resultados..." << endl;

        //guarda tiempo de ejecucion en csv
        ofstream fout;
        fout.open(path+"res/tiempos.csv", ios_base::app);
        fout << vocabulario_size << ",parallel," << duration << "\n";
        
        //guardamos la matriz aplanada en csv
        matrix_to_CSV("bow-p-matrix.csv", matriz_aplanada, libros_size, vocabulario_size);
        
        //guardamos el dataframe en csv
        dataframe_to_CSV("bow-p-df.csv", libros, libros_size, vocabulario, vocabulario_size, matriz_aplanada);
    }

    
    return 420;

}