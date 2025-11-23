#include <iostream>   // rozwiazanie poprawne; prosze tylko uzupelnic o komentarz do zadania nr 6
#include <string>

using namespace std;


class Pojazd1 {
    public:
        Pojazd1() {}

        ~Pojazd1() {}

        virtual void przyspiesz() {
            cout << "Przyspieszam" << endl;
        }

        void zatrzymaj() {
            cout << "Zatrzymuje" << endl;
        }
};

class Samochod1 : public Pojazd1 {
    public:
        Samochod1() {}

        ~Samochod1() {}

        void przyspiesz() override {
            cout << "Samochod przyspiesza" << endl;
        }
};

class Rower1 : public Pojazd1 {
    public:
        Rower1() {}

        ~Rower1() {}

        void przyspiesz() override {
            cout << "Rower przyspiesza" << endl;
        }
};

class Pojazd2 {
    public:
        Pojazd2() {}

        ~Pojazd2() {}

        virtual void przyspiesz() = 0;

        void zatrzymaj() {
            cout << "Zatrzymuje" << endl;
        }
};

class Samochod2 : public Pojazd2 {
    public:
        Samochod2() {}

        ~Samochod2() {}

        void przyspiesz() override {
            cout << "Samochod przyspiesza" << endl;
        }
};

class Rower2 : public Pojazd2 {
    public:
        Rower2() {}

        ~Rower2() {}

        void przyspiesz() override {
            cout << "Rower przyspiesza" << endl;
        }
};

class Pojazd5 {
    protected:
        string marka;
        string model;
        Pojazd5(string mrk, string mdl) : marka(mrk), model(mdl) {}
    public:
        ~Pojazd5() {}

        virtual void przyspiesz() {
            cout << "Przyspieszam" << endl;
        }

        void zatrzymaj() {
            cout << "Zatrzymuje" << endl;
        }
};

class Samochod5 : protected Pojazd5 {
    public:
        Samochod5(string mrk, string mdl) : Pojazd5(mrk, mdl) {}

        ~Samochod5() {}

        void przyspiesz() override {
            cout << "Samochod " << marka << " " << model << " przyspiesza" << endl;
        }
};

class Pojazd6 {
    protected:
        string marka;
        string model;
        Pojazd6(string mrk, string mdl) : marka(mrk), model(mdl) {}
    public:
        ~Pojazd6() {}

        virtual void przyspiesz() {
            cout << "Przyspieszam" << endl;
        }

        void zatrzymaj() {
            cout << "Zatrzymuje" << endl;
        }
};

class Samochod6 : private Pojazd6 {
    public:
        Samochod6(string mrk, string mdl) : Pojazd6(mrk, mdl) {}

        ~Samochod6() {}

        void przyspiesz() override {
            cout << "Samochod " << marka << " " << model << " przyspiesza" << endl;
        }
};
// jaka jest zatem roznica miedzy dziedziczeniem protected a private?

class Silnikowy7 {
    public:
        Silnikowy7() {}

        ~Silnikowy7() {}

        virtual void przyspiesz() {
            cout << "Silnikowe przyspieszenie" << endl;
        }
};

class Elektryczny7 {
    public:
        Elektryczny7() {}

        ~Elektryczny7() {}

        void ladowanie() {
            cout << "Elektryczne ladowanie" << endl;
        }
};

class Hybryda7 : public Silnikowy7, public Elektryczny7 {
    public:
        Hybryda7() {}

        ~Hybryda7() {}

        void przyspiesz() override {
            cout << "Hybrydowe przyspieszanie" << endl;
            ladowanie();
        }
};

class Pojazd9 {
    public:
        Pojazd9() {}

        ~Pojazd9() {}

        virtual void przyspiesz() {
            cout << "Przyspieszam" << endl;
        }

        void zatrzymaj() {
            cout << "Zatrzymuje" << endl;
        }
};

class Samochod9 : public Pojazd9 {
    public:
        Samochod9() {}

        ~Samochod9() {}

        void przyspiesz() override {
            cout << "Samochod przyspiesza" << endl;
        }
};

class ElektrycznySamochod9 : public Samochod9 {
    public:
        ElektrycznySamochod9() {}

        ~ElektrycznySamochod9() {}

        void przyspiesz() override {
            cout << "Elektryczny samochod przyspiesza" << endl;
        }
        
        void ladowanie() {
            cout << "Elektryczny samochod laduje" << endl;
        }
};

class Pojazd0 {
    public:  // szkoda, że nie jeszcze atrybutow w klasie
        Pojazd0() {}

        ~Pojazd0() {}

        virtual void przyspiesz() {
            cout << "Przyspieszam" << endl;
        }

        void zatrzymaj() {
            cout << "Zatrzymuje" << endl;
        }
};

class Silnikowy0 : virtual public Pojazd0 {
    public:
        Silnikowy0() {}

        ~Silnikowy0() {}

        void przyspiesz() override {
            cout << "Silnikowe przyspieszenie" << endl;
        }
};

class Elektryczny0 : virtual public Pojazd0 {
    public:
        Elektryczny0() {}

        ~Elektryczny0() {}

        void ladowanie() {
            cout << "Elektryczne ladowanie" << endl;
        }

        void przyspiesz() override {
            cout << "Elektryczne przyspieszenie" << endl;
        }
};

class Hybryda0 : public Silnikowy0, public Elektryczny0 {
    public:
        Hybryda0() {}

        ~Hybryda0() {}

        void przyspiesz() override {
            cout << "Hybrydowe przyspieszanie" << endl;
            ladowanie();
        }
};


void Zadanie1() {
    Pojazd1 pojazd1;
    Samochod1 samochod1;
    Rower1 rower1;

    pojazd1.przyspiesz(); 
    pojazd1.zatrzymaj();

    samochod1.przyspiesz(); 
    samochod1.zatrzymaj();  

    rower1.przyspiesz(); 
    rower1.zatrzymaj();

    pojazd1.przyspiesz();
    pojazd1.zatrzymaj();
}

void Zadanie2() {
    Samochod2 samochod2;
    Rower2 rower2;

    samochod2.przyspiesz(); 
    samochod2.zatrzymaj();  

    rower2.przyspiesz(); 
    rower2.zatrzymaj();
}

void Zadanie3() {
    Pojazd1 pojazd3;
    Samochod1 samochod3;
    Rower1 rower3;

    pojazd3.przyspiesz(); 
    pojazd3.zatrzymaj();

    samochod3.przyspiesz(); 
    samochod3.zatrzymaj();  

    rower3.przyspiesz(); 
    rower3.zatrzymaj();

    pojazd3.przyspiesz();
    pojazd3.zatrzymaj();
}

void Zadanie5() {
    Samochod5 samochod5("Bugatti", "Chiron");
    samochod5.przyspiesz();
}

void Zadanie6() {
    Samochod6 samochod6("Bugatti", "Chiron");
    samochod6.przyspiesz();
}

void Zadanie7() {
    Hybryda7 hybryda7;
    hybryda7.przyspiesz();
}

void Zadanie9() {
    ElektrycznySamochod9 elektrycznysamochod9;
    Samochod9 samochod9;
    Pojazd9 pojazd9;

    pojazd9.przyspiesz();
    pojazd9.zatrzymaj();

    samochod9.przyspiesz();
    samochod9.zatrzymaj();

    elektrycznysamochod9.przyspiesz();
    elektrycznysamochod9.zatrzymaj();
    elektrycznysamochod9.ladowanie();

    samochod9.przyspiesz();
    samochod9.zatrzymaj();

    pojazd9.przyspiesz();
    pojazd9.zatrzymaj();
}

void Zadanie0() {
    Hybryda0 hybryda0;
    Pojazd0 pojazd0;
    Elektryczny0 elektryczny0;
    Silnikowy0 silnikowy0;

    pojazd0.przyspiesz();
    pojazd0.zatrzymaj();

    silnikowy0.przyspiesz();
    silnikowy0.zatrzymaj();

    elektryczny0.przyspiesz();
    elektryczny0.zatrzymaj();
    elektryczny0.ladowanie();

    hybryda0.przyspiesz();
    hybryda0.zatrzymaj();
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
            cout << "Zadanie w Javie" << endl;
            break;
        case 5:
            cout << "Zadanie 5" << endl;
            Zadanie5();
            break;
        case 6:
            cout << "Zadanie 6" << endl;
            Zadanie6();
            break;
        case 7:
            cout << "Zadanie 7" << endl;
            Zadanie7();
            break;
        case 8:
            cout << "Zadanie w Javie" << endl;
            break;
        case 9:
            cout << "Zadanie 9" << endl;
            Zadanie9();
            break;
        case 10:
            cout << "Zadanie 10" << endl;
            Zadanie0();
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
        cout << "\nWybierz zadanie od 1 do 10 (0 aby zakończyć): ";
        cin >> number;
        WybórZadania(number);
    } while (number != 0);

    return 0;

}
