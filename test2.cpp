#include <map>
#include <string>
#include <iostream>
#include <fstream>
#include <cmath>
#include <mpi.h>


using namespace std;

int main(int argc, char ** argv){
    
    cout << "Hola mundo\n";
    //numero de libros
    const int libros_size = 6;
    //array de strings con los nombres de los libros
    string libros[libros_size] = {"shakespeare_the_merchant_of_venice", "shakespeare_romeo_juliet", "shakespeare_hamlet",
                         "dickens_a_christmas_carol", "dickens_oliver_twist", "dickens_a_tale_of_two_cities"};
    //tamano del vocabulario
    const long long int vocabulario_size = 15164;//atoi(argv[1]);
    //array de strings con el vocabulario
    string* vocabulario = new string[vocabulario_size];
    //diccionarios de cada libro
    map<string, int> diccionarios[libros_size];

    //matriz donde se guardan los resultados
    int matriz_aplanada[libros_size*vocabulario_size]{0};
    int* matriz[libros_size][vocabulario_size]{0};

    //array auxiliar donde se guardan los resultados de cada proceso
    int aux[vocabulario_size]{0};

    int num_processes;
	int process_id = 0;

    string file_name = "vocabulario.txt";

    cout << "Leyendo vocabulario\n";

    //load_vocabulary("vocabulario.txt", vocabulario, vocabulario_size);
    {
    //carga el vocabulario en el array de strings
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
    cout << "Iniciando diccionarios\n";
    for (long long int i = 0; i < libros_size; i++) {
        for (long long int j = 0; j < vocabulario_size; j++) {
            //recorremos el vocabulario y lo guardamos en el diccionario
            diccionarios[i].insert({vocabulario[j], 0});
        }
    }   

    MPI_Init(&argc, &argv);

    MPI_Comm_size(MPI_COMM_WORLD, &num_processes);
	MPI_Comm_rank(MPI_COMM_WORLD, &process_id);

    //count_parallel_words()
    {
    ifstream book("O:/itam/ComParalelo/Parallel-bag-of-words/data/"+libros[process_id]+".txt");
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
    }
    cout << "imprimo diccionario\n";

    //imprime el diccionario
    cout << "libro " << libros[process_id] << endl; 
    for (long long int i = 0; i < 10; i++) {
        cout << diccionarios[process_id][vocabulario[i]] << " ";
    } 
    cout << endl;

    cout << "termino diccionario" << endl;

    

    for (long long int j = 0; j < vocabulario_size; j++) {
        //va guardanddo los valores de cada diccionario en el array auxiliar
        aux[j] = diccionarios[process_id][vocabulario[j]];
    }
    cout << "imprimo aux\n";
    
    cout << "libro " << libros[process_id] << endl; 
    for (long long int i = 0; i < 10; i++) {
        cout << aux[i] << " ";
    }

    cout << "\ntermino aux" << endl;

    cout << "\n\nGather\n" << endl;
    //Juntamos los resultados de cada proceso
    MPI_Gather(aux, 
            vocabulario_size, //cada array tiene el tamaño del vocabulario
            MPI_INT, //manda la cuenta de cada palabra
            matriz_aplanada, //matriz donde se guardan los resultados
            vocabulario_size, //enviamos lo mismo que recibimos
            MPI_INT, //x2
            1, //donde lo recibimos 
            MPI_COMM_WORLD);

    

    //si es el proceso 0 imprime la matriz
    if (process_id == 1) {
        cout << "imprimo matriz\n";
        for (long long int i = 0; i < libros_size; i++) {
            cout << "libro " << libros[i] << endl; 
            for (long long int j = 0; j < 10; j++) {
                cout << matriz_aplanada[i*vocabulario_size+j] << " ";
            }
            cout << endl;
        }
        // for(int i = 0; i < libros_size; i++){
        //     cout << "libro " << libros[i] << endl; 
        //     for (long long int j = 0; j < 10; j++) {
        //         cout << matriz[i][j] << " ";
        //     } 
        //     cout << endl;
        // }
        
    }



    MPI_Finalize();

    cout << "Fin imprimiendo\n";

    return 420;

    

}