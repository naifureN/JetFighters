#include <SFML/Graphics.hpp>
#include <random>
#include <iostream>
#include <unordered_map>



int main() {
    sf::RenderWindow window(sf::VideoMode(900, 900), "Jet Fighters");
    sf::Event event;
    sf::Clock clock;

    while (window.isOpen()) {
        int deltaTime = clock.getElapsedTime().asMicroseconds();
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

        }

        clock.restart();
        window.clear();
        window.display();
    }
    return 0;
}
