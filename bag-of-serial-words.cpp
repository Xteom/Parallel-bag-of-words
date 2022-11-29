#include <map>
#include <string>
#include <iostream>
#include <fstream>
#include <cmath>

using namespace std;


//carga el vocabulario en un array de strings
//para simplificar el código el vocabulario y su size se obtienen del jupyter notebook
void load_vocabulary(string file_name, string* vocabulario, long long int vocabulario_size) {
    ifstream in("data/"+file_name);
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
void count_words(string* libros, long long int libros_size, 
                map<string, int>* diccionarios) {
    for (long long int i = 0; i < libros_size; i++) {
        ifstream in("data/"+libros[i]+".txt");
        if (!in) {
            cerr << "Couldn't read file: " << libros[i] << "\n";
        }
        while (!in.eof()) {
            string word;
            //lee coma por coma y lo guarda en word
            getline(in, word, ',');
            //si la palabra esta en el vocabulario
            if (diccionarios[i].count(word) > 0) {
                //le suma 1 a su contador
                diccionarios[i][word] = diccionarios[i][word] + 1;
            }
        }
    }
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
 
    //numero de libros
    const int libros_size = 6;
    //array de strings con los nombres de los libros
    string libros[libros_size] = {"shakespeare_the_merchant_of_venice", "shakespeare_romeo_juliet", "shakespeare_hamlet", "dickens_a_christmas_carol", "dickens_oliver_twist", "dickens_a_tale_of_two_cities"};
    //tamano del vocabulario
    const long long int vocabulario_size = atoi(argv[1]);
    //array de strings con el vocabulario
    string* vocabulario = new string[vocabulario_size];
    //diccionarios de cada libro
    map<string, int> diccionarios[libros_size];
    
    //carga el vocabulario en el array de strings
    load_vocabulary("vocabulario.txt", vocabulario, vocabulario_size);

    //inicializa los diccionarios con el vocabulario
    for (long long int i = 0; i < libros_size; i++) {
        for (long long int j = 0; j < vocabulario_size; j++) {
            diccionarios[i].insert({vocabulario[j], 0});
        }
    }

    //cuenta palabras de los libros
    count_words(libros, libros_size, diccionarios);

    //guarda la matriz en csv
    matrix_to_CSV("bow-s-matrix.csv", diccionarios, libros_size, vocabulario, vocabulario_size);

    //guarda el dataframe en csv
    dataframe_to_CSV("bow-s-df.csv", diccionarios, libros_size, vocabulario, vocabulario_size, libros);

    return 420;

}