#include <iostream>   // rozwiazanie poprawne
#include <string>
#include <vector>

using namespace std;


class Car1 {
    private:
        string make;
        string model;
        int year;
    public:
        Car1(string mk, string mdl, int yr) : make(mk), model(mdl), year(yr) {
            cout << "\nStworzono konstruktor" << endl;
        }

        void displayInfo() const {
            cout << "\nMarka: " << make << "\nModel: " << model << "\nRok: " << year << endl;
        }
};

class Car2 {
    private:
        string make;
        string model;
        int year;
        int currentYear = 2025;
    public:
        Car2(string mk, string mdl, int yr) : make(mk), model(mdl), year(yr) {
            cout << "\nStworzono konstruktor" << endl;
        }

        void displayInfo() const {
            cout << "\nMarka: " << make << "\nModel: " << model << "\nRok: " << year << endl;
        }

        int getCarAge() const {
            return currentYear - year;
        }
};

class Car3 {
    private:
        string make;
        string model;
        int year;
        int currentYear = 2025;
    public:
        Car3(string mk, string mdl, int yr) : make(mk), model(mdl), year(yr) {
            cout << "\nStworzono konstruktor" << endl;
        }

        void displayInfo() const {
            cout << "\nMarka: " << make << "\nModel: " << model << "\nRok: " << year << endl;
        }

        int getCarAge() const {
            return currentYear - year;
        }

        string getMake() const {
            return make;
        }

        string getModel() const {
            return model;
        }

        int getYear() const {
            return year;
        }
};

bool isSameCar(const Car3& Car, const Car3& otherCar) {
    if ((Car.getMake() == otherCar.getMake()) && (Car.getModel() == otherCar.getModel()) && (Car.getYear() == otherCar.getYear())) {
        cout << "To jest ten sam samochód." << endl;
        return true;
    } else {
        cout << "To nie jest ten sam samochód." << endl;  // wlasciwie "taki sam"
        return false;
    }
}

class Parrot {
    private:
        string name;
        vector<string> sentance = {"Hello World!", "Witaj świecie!"};
    public:

        Parrot(string n, string sen) : name(n) {
            sentance.push_back(sen);
            cout << "Stworzono papugę o imieniu " << name << ", która mówi: " << sentance[sentance.size() - 1] << endl;
        }

        void changeSentance(string newSentance) {   // raczej dodanie frazy, nie zmiana
            sentance.push_back(newSentance);
            cout << name << " mówi: " << newSentance << endl;
        }

        void say(int n) {
            for (int i = 0; i < n; i++) {
                int random = rand() % (sentance.size());
                cout << name << " mówi: " << sentance[random] << endl;
            }
        }
};


void Zadanie1() {
    Car1 CarIwouldLikeToHave1("Bugatti", "Chiron", 2016);
    CarIwouldLikeToHave1.displayInfo();

    /*###############################################################
    
    1. Ponieważ  specyfikator dostępu private czyni atrybuty dostepnymi tylko z poziomu danej klasy, co zwiększa bezpieczeństwo.
    2. Jeśli zmienimy private na public to atrybuty beą dostępne z dowolnego miejsca w kodzie.
    3. Można utworzyć dowolną ilość obiektów klasy Car. Atrybuty są osobnymi zmiennymi dla każdego obiektu klasy (nawet jeśli się tak samo nazywają).

    ###############################################################*/
}

void Zadanie2() {
    Car2 CarIwouldLikeToHave2("Ferrari", "488 GT3", 2015);
    CarIwouldLikeToHave2.displayInfo();
    int age = CarIwouldLikeToHave2.getCarAge();
    cout << "Samochód ma " << age << " lat" << endl;
}

void Zadanie3() {
    Car3 CarIwouldLikeToHave3("Ferrari", "488 GT3", 2015);
    Car3 CarIwouldLikeToHave4("Ferrari", "488 GT3", 2015);
    isSameCar(CarIwouldLikeToHave3, CarIwouldLikeToHave4);
}

void Zadanie4() {
    srand(time(0));

    Parrot papug("Henio", "Hallo Welt!");
    papug.say(3);
    papug.changeSentance("Bonjour le monde!");
    papug.say(3);
}


//Funkcja ze switch case'm do wyboru odpowiedniego zadania.
void WybórZadania(int number) {
    switch (number) {  // ladne menu wyboru
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
