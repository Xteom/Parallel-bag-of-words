#include <map>
#include <string>
#include <iostream>
#include <fstream>
#include <cmath>

using namespace std;



void load_vocabulary(string file_name, string* vocabulario, long long int size) {
    ifstream in("data/"+file_name);
    if (!in) {
        cerr << "Couldn't read file: " << file_name << "\n";
    }
    long long int word_number = 0;
    while (!in.eof() && (word_number < size)) {
        getline(in, vocabulario[word_number], '\n');
        word_number++;
    }
}

//convierte la matriz de points en un archivo csv
void save_to_CSV(string file_name, float** points, long long int size) {
    fstream fout;
    fout.open("dump/"+file_name, ios::out);
    for (long long int i = 0; i < size; i++) {
        fout << points[i][0] << ","
             << points[i][1] << ","
             << points[i][2] << "\n";
    }
}

int main(int argc, char ** argv){

    const long long int size = atoi(argv[1]);
    string* vocabulario = new string[size];

    load_vocabulary("vocabulario.txt", vocabulario, size);

    for (long long int i = 0; i < size; i++) {
        cout << vocabulario[i] << "\n";
    }




    return 0;

}