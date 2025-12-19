#include <iostream>
#include <string>
#include <vector>

using namespace std;

class Kurs;

class Osoba {
    protected:
        string imie;
        string nazwisko;
        int dataUrodzenia;

    public:
        Osoba(string i, string n, int d) : imie(i), nazwisko(n), dataUrodzenia(d) {}

        virtual ~Osoba() {}
};

class Uczen : public Osoba {
    private:
        vector<Kurs*> kursy;

    public:
        Uczen(string i, string n, int d) : Osoba(i, n, d) {}

        void zapiszNaKurs(Kurs* k) {
            kursy.push_back(k);
        }
};

class Nauczyciel : public Osoba {
    private:
        vector<Kurs*> prowadzoneKursy;

    public:
        Nauczyciel(string i, string n, int d) : Osoba(i, n, d) {}

        void dodajKurs(Kurs* k) {
            prowadzoneKursy.push_back(k);
        }
};

class SalaLekcyjna {
    private:
        int numerSali;
        int pojemnosc;

    public:
        SalaLekcyjna(int nr, int poj) : numerSali(nr), pojemnosc(poj) {}
};

class Material {
    private:
        string nazwaMaterialu;
        string typPliku;

    public:
        Material(string nazwa, string typ) : nazwaMaterialu(nazwa), typPliku(typ) {}
};

class Kurs {
    private:
        string nazwaKursu;
        int kodKursu;
        Nauczyciel* nauczyciel;
        SalaLekcyjna* sala;
        vector<Uczen*> uczniowie;
        vector<Material> materialy;

    public:
        Kurs(string nazwa, int kod, SalaLekcyjna* s) : nazwaKursu(nazwa), kodKursu(kod), sala(s) {}

        void przypiszNauczyciela(Nauczyciel* n) {
            nauczyciel = n;
            n->dodajKurs(this);
        }

        void dodajUcznia(Uczen* u) {
            uczniowie.push_back(u);
            u->zapiszNaKurs(this);
        }

        void dodajMaterial(string nazwa, string typ) {
            materialy.emplace_back(nazwa, typ);
        }
};

int main() {
    SalaLekcyjna sala1(42, 30);

    Nauczyciel nauczyciel1("Vesemir", "z Kaer Morhen", 1960);
    Uczen uczen1("Geralt", "z Rivii", 2005);
    Uczen uczen2("Ciri", "z Cintry", 2006);

    Kurs kurs1("Historia Magii", 101, &sala1);

    kurs1.przypiszNauczyciela(&nauczyciel1);
    kurs1.dodajUcznia(&uczen1);
    kurs1.dodajUcznia(&uczen2);

    kurs1.dodajMaterial("Kompedium Wiedzy", "Papirus");

    return 0;
}

// Role specjalne: Nauczyciel, Uczeń

/*##################  Digaram UML  ##################


classDiagram

class Osoba {
- imie : string
- nazwisko : string
- dataUrodzenia : int
+ Osoba()
+ ~Osoba()
}

class Nauczyciel {
- prowadzoneKursy : vector
+ dodajKurs()
}

class Uczen {
- kursy : vectro
+ zapiszNaKurs()
}

class Kurs {
- nazwaKursu : string
- kodKursu : int
+ Kurs()
+ przypiszNauczyciela()
+ dodajUcznia()
+ dodajMaterial()
}

class SalaLekcyjna {
- numerSali : int
- pojemnosc : int
+ SalaLekcyjna()
}

class Material {
- nazwaMaterialu : string
- typPliku : string
+ Material()
}

%% Dziedziczenie
Osoba <|-- Nauczyciel
Osoba <|-- Uczen

%% Asocjacja (wiele do wielu)
Uczen "0..*" -- "0..*" Kurs

%% Asocjacja (nauczyciel prowadzi kursy)
Nauczyciel "1" -- "0..*" Kurs

%% Relacja zwykła
%%Kurs "1" --> "1" SalaLekcyjna

%% Kompozycja
Kurs *-- "0..*" Material

%% Agregacja
SalaLekcyjna --o "1.." Kurs


####################################################*/
