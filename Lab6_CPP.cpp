#include <iostream>    // Rozwiazanie poprawne i wyczerpujace; wpisuje "+"
#include <string>
#include <vector>
#include <memory>

using namespace std;


class Pojazd {
    protected:
        string marka;
    public:
        Pojazd(string m) : marka(m) {}

        virtual ~Pojazd() {}

        virtual void przyspiesz() {
            cout << marka << " przyspiesza" << endl;
        }

        virtual void zatrzymaj() {
            cout << marka << " zatrzymuje się" << endl;
        }
};

class Samochod : public Pojazd {
    public:
        Samochod(string mrk) : Pojazd(mrk) {}

        ~Samochod() {}

        void przyspiesz() override {
            cout << "Samochod " << marka << " przyspiesza" << endl;
        }

        void zatrzymaj() override {
            cout << "Samochod " << marka << " zatrzymuje się" << endl;
        }
};

class Rower : public Pojazd {
    public:
        Rower(string mrk) : Pojazd(mrk) {}

        ~Rower() {}

        void przyspiesz() override {
            cout << "Rower " << marka << " przyspiesza" << endl;
        }

        void zatrzymaj() override {
            cout << "Rower " << marka << " zatrzymuje się" << endl;
        }
};

class Motocykl : public Pojazd {
    public:
        Motocykl(string mrk) : Pojazd(mrk) {}

        ~Motocykl() {}

        void przyspiesz() override {
            cout << "Motocykl " << marka << " przyspiesza" << endl;
        }

        void zatrzymaj() override {
            cout << "Motocykl " << marka << " zatrzymuje się" << endl;
        }
};

class Skuter : public Pojazd {
    public:
        Skuter(string mrk) : Pojazd(mrk) {}

        ~Skuter() {}

        void przyspiesz() override {
            cout << "Skuter " << marka << " przyspiesza" << endl;
        }
        
        void zatrzymaj() override {
            cout << "Skuter " << marka << " zatrzymuje się" << endl;
        }
};

void Zadanie1() {
    Pojazd pojazd1("Pojazd");

    pojazd1.przyspiesz(); 
    pojazd1.zatrzymaj();
}

void Zadanie2() {
    Pojazd pojazd2("Pojazd");
    Samochod samochod2("Bugatti Chiron");
    Rower rower2("BMX");
    Motocykl motocykl2("Harley");

    pojazd2.przyspiesz(); 
    pojazd2.zatrzymaj();

    samochod2.przyspiesz(); 
    samochod2.zatrzymaj();  

    rower2.przyspiesz(); 
    rower2.zatrzymaj();

    motocykl2.przyspiesz();
    motocykl2.zatrzymaj();

    pojazd2.przyspiesz();
    pojazd2.zatrzymaj();
}

void Zadanie3() {
    vector<unique_ptr<Pojazd>> przechowalnia1;

    przechowalnia1.push_back(make_unique<Samochod>("Bugatti Chiron"));
    przechowalnia1.push_back(make_unique<Rower>("BMX"));
    przechowalnia1.push_back(make_unique<Motocykl>("Harley"));
    przechowalnia1.push_back(make_unique<Samochod>("Tesla Model S"));

    for (int i=0; i<przechowalnia1.size(); i++) {
        przechowalnia1[i]->przyspiesz();
        przechowalnia1[i]->zatrzymaj();

        cout << endl;
    }
}

void Zadanie4() {
    vector<unique_ptr<Pojazd>> przechowalnia2;

    przechowalnia2.push_back(make_unique<Samochod>("Bugatti Chiron"));
    przechowalnia2.push_back(make_unique<Rower>("BMX"));
    przechowalnia2.push_back(make_unique<Motocykl>("Harley"));
    przechowalnia2.push_back(make_unique<Samochod>("Tesla Model S"));
    przechowalnia2.push_back(make_unique<Skuter>("Vespa"));

    for (int i=0; i<przechowalnia2.size(); i++) {
        przechowalnia2[i]->przyspiesz();
        przechowalnia2[i]->zatrzymaj();

        cout << endl;
    }
}

void WybórZadania(int number) {
    switch (number) {
        case 0:
            cout << "Zamykanie programu" << endl;
            break;
        case 1:
            cout << "Zadanie 1" << endl;
            Zadanie1();
            break;
        case 2:
            cout << "Zadanie 2" << endl;
            Zadanie2();
            break;
        case 3:
            cout << "Zadanie 3" << endl;
            Zadanie3();
            break;
        case 4:
            cout << "Zadanie 4" << endl;
            Zadanie4();
            break;
        default:
            cout << "Wybrano nieprawidłową opcje." << endl;
            break;
    }
}


int main() {
    int number;

    do
    {
        cout << "\nWybierz zadanie od 1 do 4 (0 aby zakończyć): ";
        cin >> number;
        WybórZadania(number);
    } while (number != 0);

    return 0;

}

