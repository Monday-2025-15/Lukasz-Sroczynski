#include <iostream>
#include <string>

using namespace std;

class Osoba {
    private:
        string imie;
        string nazwisko;
        int dataUrodzenia;
    public:
        Osoba(string i, string n, int d) : imie(i), nazwisko(n), dataUrodzenia(d) {
            cout << "Stworzono osobę: " << imie << " " << nazwisko << ", data urodzenia: " << dataUrodzenia << endl;
        }

        ~Osoba() {
            cout << "Usunięto osobę: " << imie << " " << nazwisko << endl;
        }
};

class Nauczyciel : public Osoba {
    private:
        string kurs;
    public:
        Nauczyciel(string i, string n, int d, string k) : Osoba(i, n, d), kurs(k) {
            cout << "Stworzono nauczyciela kursu: " << kurs << endl;
        }

        ~Nauczyciel() {
            cout << "Usunięto nauczyciela kursu: " << kurs << endl;
        }
};

class Uczen : public Osoba {
    private:
        string kurs;
    public:
        Uczen(string i, string n, int d, string k) : Osoba(i, n, d), kurs(k) {
            cout << "Stworzono ucznia w kursie: " << kurs << endl;
        }

        ~Uczen() {
            cout << "Usunięto ucznia w kursie: " << kurs << endl;
        }
};

class kurs {
    private:
        string nazwaKursu;
        int kodKursu;
    public:
        kurs(string nazwa, int kod) : nazwaKursu(nazwa), kodKursu(kod) {
            cout << "Stworzono kurs: " << nazwaKursu << ", o kodie " << kodKursu << endl;
        }

        ~kurs() {
            cout << "Usunięto kurs: " << nazwaKursu << endl;
        }
};

class salaLekcyjna {
    private:
        int numerSali;
        int pojemnosc;
    public:
        salaLekcyjna(int nr, int poj) : numerSali(nr), pojemnosc(poj) {
            cout << "Stworzono salę lekcyjną nr: " << numerSali << ", o pojemności " << pojemnosc << endl;
        }

        ~salaLekcyjna() {
            cout << "Usunięto salę lekcyjną nr: " << numerSali << endl;
        }
};

class material {
    private:
        string nazwaMaterialu;
        string typPliku;
    public:
        material(string nazwa, string typ) : nazwaMaterialu(nazwa), typPliku(typ) {
            cout << "Stworzono materiał: " << nazwaMaterialu << ", typu: " << typPliku << endl;
        }

        ~material() {
            cout << "Usunięto materiał: " << nazwaMaterialu << endl;
        }
};


int main() {
    Osoba osoba1("Lambert", "Z Łodzi", 1980);
    Nauczyciel nauczyciel1("Visemir", "Z Norymbergii", 1975, "Historia Sztuki Współczesnej");
    Uczen uczen1("Geral", "Z Rivii", 2005, "Historia Sztuki Współczesnej");
    kurs kurs1("Historia Sztuki Współczesnej", 101);
    salaLekcyjna sala1(42, 30);
    material material1("Kompedium Wiedzy", "Papirus");

    return 0;
}

// Role specjalne: Nauczyciel, Uczeń

/*##################  Digaram UML  ##################


classDiagram

class Osoba {   
- string imie
- string nazwisko
- int dataUrodzenia
- Osoba(imie, nazwisko, dataUrodzenia)
- ~Osoba()
}

class Nauczyciel {   
- string kurs
- Nauczyciel(imie, nazwisko, dataUrodzenia, kurs)
- ~Nauczyciel()
}

class Uczen {
- string kurs
- Uczen(imie, nazwisko, dataUrodzenia, kurs)
- ~Uczen()
}

class kurs {
- string nazwaKursu
- int kodKursu
- kurs(nazwaKursu, kodKursu)
- ~kurs()
}

class salaLekcyjna {
- int numerSali
- int pojemnosc
- salaLekcyjna(numerSali, pojemnosc)
- ~salaLekcyjna()
}

class material {    
- string nazwaMaterialu
- string typPliku
- material(nazwaMaterialu, typPliku)
- ~material()
}


%% Dziedziczenie
Osoba <|-- Nauczyciel
Osoba <|-- Uczen

%% Asocjacja:
Uczen --> "1.." kurs
kurs --> "0.." Uczen

%% Agregacja:
Nauczyciel o-- "0.." kurs
kurs --> "0..1" Nauczyciel

%% Kompozycja:
kurs *-- "0.." material
material --> "1" kurs

%% Relacja zwykła:
kurs --> "1" salaLekcyjna
salaLekcyjna --> "0.." kurs


####################################################*/