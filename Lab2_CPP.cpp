#include <iostream>
#include <string>

using namespace std;


class Samochód {
    private:
        string marka;
        string model;
        int rokProdukcji;
    public:
        Samochód() : marka("-"), model("-"), rokProdukcji(0) {
            cout << "Domyślny konstruktor wywołany." << endl;
        }

        Samochód(string mk, string mdl, int yr) : marka(mk), model(mdl), rokProdukcji(yr) {
            cout << "\nKonstruktor wywołany" << endl;
        }

        ~Samochód() {
            cout << "Destruktor wywołany dla " << marka << " " << model << endl;
        }
};


class Tablica {
    private:
        int* arr;
        int size;
    public:
        Tablica() : size(10) {
            arr = new int[size];
            cout << "Domyślny konstruktor wywołany" << endl;
        }

        Tablica(int s) : size(s) {
            arr = new int[size];
            cout << "Konstruktor wywołany" << endl;
        }

        ~Tablica() {
            delete[] arr;
            cout << "Destruktor wywołany dla tablicy o rozmiarze " << size << endl;
        }
};


int main() {
    cout << "Zadanie 1:" << endl;
    Samochód car1;
    Samochód car2("Buggati", "Chiron", 2020);
    Samochód car3("Ferrari", "488 GT3", 2015);

    cout << "Zadanie 2:" << endl;
    Tablica arr1;
    Tablica arr2(20);
    Tablica arr3(50);
    return 0;
}