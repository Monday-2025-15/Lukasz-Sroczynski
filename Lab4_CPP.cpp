#include <iostream>
#include <string>

using namespace std;



class Osoba {
private:
    string imie;
    string nazwisko;
    int wiek;
    
    string email;
    string telefon;

public:
    string getImie() {
        return imie;
    }

    string getNazwisko() {
        return nazwisko;
    }

    int getWiek() {
        return wiek;
    }

    void setImie(string noweImie) {
        imie = noweImie;
    }

    void setNazwisko(string noweNazwisko) {
        nazwisko = noweNazwisko;
    }

    void setWiek(int nowyWiek) {
        wiek = nowyWiek;
    }

    string getEmail() {
        return email;
    }

    string getTelefon() {
        return telefon;
    }

    void setEmail(string nowyEmail) {
        if (nowyEmail.find('@') != string::npos) {
            email = nowyEmail;
        } else {
            cout << "Wprowadzono niepoprawny adres email." << endl;
        }
    }

    void setTelefon(string nowyTelefon) {
        if (nowyTelefon.length() == 9) {
            telefon = nowyTelefon;
        } else {
            cout << "Wprowadzono niepoprawny numer telefonu" << endl;
        }
    }
};


class Pracownik {
protected:
    string stanowisko;
    float wynagrodzenie;

public:
    Pracownik(string s, float w) : stanowisko(s), wynagrodzenie(w) {
        cout << "Stworzono pracownika" << endl;
    }

    ~Pracownik() {
       cout << "Zniszczono pracownika" << endl; 
    }
};

class Nauczyciel : public Pracownik {
private:
    string przedmiot;

public:
    Nauczyciel(string s, float w, string p) : Pracownik(s, w), przedmiot(p) {
        cout << "Stworzono nauczyciela" << endl;
    }
    
    ~Nauczyciel() {
         cout << "Zniszczono nauczyciela" << endl;
    }

    void pokazDane() {
        cout << "\nStanowisko: " << stanowisko << "   Wynagrodzenie: " << wynagrodzenie << "zł   Nauczany przedmiot: " << przedmiot << endl;
    }
};

class Administracja : public Pracownik {
private:
    string dzial;

public:
    Administracja(string s, float w, string d) : Pracownik(s, w), dzial(d) {
        cout << "Stworzono administratora" << endl;
    }

    ~Administracja() {
         cout << "Zniszczono administratora" << endl;
    }

    void pokazDane() {
        cout << "\nStanowisko: " << stanowisko << "   Wynagrodzenie: " << wynagrodzenie << "zł   Dział: " << dzial << endl;
    }
};



int main() {
    Osoba czlowiek;

    czlowiek.setImie("Jan");
    czlowiek.setNazwisko("Paweł");
    czlowiek.setWiek(2);

    cout << "\nTestowanie niepoprawnych danych:" << endl;
    czlowiek.setEmail("jan.pawel.com");
    czlowiek.setTelefon("6942069");
    cout << "\nTestowanie poprawnych danych:" << endl;
    czlowiek.setEmail("jan.pawel@onet.pl");
    czlowiek.setTelefon("123456789");

    cout << "Imie: " << czlowiek.getImie() << "   Nazwisko: " << czlowiek.getNazwisko() << "   Wiek: " << czlowiek.getWiek() << "   Email: " << czlowiek.getEmail() << "   Telefon: " << czlowiek.getTelefon() << endl;



    Nauczyciel nauczyciel("Nauczyciel biologii", 3601.50, "Informatyka");
    Administracja administrator("Sekretarka", 5300.01, "Sekretariat");

    nauczyciel.pokazDane();
    administrator.pokazDane();

    return 0;
}