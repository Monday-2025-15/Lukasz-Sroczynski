class DatabaseConnection {   // rozwiazanie poprawne
    private static DatabaseConnection instance;

    private DatabaseConnection() {
        System.out.println("Polaczenie z baza danych utworzone");
    }

    public static DatabaseConnection getInstance() {
        if (instance == null) {
            instance = new DatabaseConnection();
        }
        return instance;
    }

    public void executeQuery(String query) {
        System.out.println("Wykonuje zapytanie: " + query);
    }
}

abstract class Vehicle {
    public abstract void drive();
}

class Car extends Vehicle {
    @Override
    public void drive() {
        System.out.println("Jade samochodem");
    }
}

class Bike extends Vehicle {
    @Override
    public void drive() {
        System.out.println("Jade rowerem");
    }
}

class VehicleFactory {    //takze i ta klasa mogła byc klasa abstrakcyjna, a w slad za nia mozna bylo zbudowac odpowiednie wytworcze klasy pochodne
    public Vehicle createVehicle(String type) {
        if (type.equals("car")) {
            return new Car();
        }
        if (type.equals("bike")) {
            return new Bike();
        }
        return null;
    }
}


class Pizza {
    private String dough;
    private String sauce;
    private String topping;

    private Pizza(PizzaBuilder builder) {
        this.dough = builder.dough;
        this.sauce = builder.sauce;
        this.topping = builder.topping;
    }

    public static class PizzaBuilder {    // tutaj z kolei mozna bylo zdefiniowac odrebny (w stosunku do klasy Pizza) interfejs, ktory bedzie nastepnie implementowany przez klasy reprezentujace przygotowanie poszczegolnych rodzajow pizzy
        private String dough;
        private String sauce;
        private String topping;

        public PizzaBuilder dough(String dough) {
            this.dough = dough;
            return this;
        }

        public PizzaBuilder sauce() {
            this.sauce = "pomidorowy";
            return this;
        }

        public PizzaBuilder topping() {
            this.topping = "ser";
            return this;
        }

        public Pizza build() {
            return new Pizza(this);
        }
    }

    @Override
    public String toString() {
        return "Pizza: ciasto=" + dough + ", sos=" + sauce + ", dodatki=" + topping;
    }
}

public class Lab8_JAVA {
    public static void main(String[] args) {

        DatabaseConnection db1 = DatabaseConnection.getInstance();
        db1.executeQuery("SELECT * FROM users");

        VehicleFactory factory = new VehicleFactory();
        Vehicle car = factory.createVehicle("car");
        car.drive();

        Pizza pizza = new Pizza.PizzaBuilder().dough("cienkie").sauce().topping().build();

        System.out.println(pizza);
    }

}

