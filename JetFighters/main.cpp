#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>

const float SPEED = 800;

sf::Vector2f normalizeVector(sf::Vector2f vect) {
    if (vect == sf::Vector2f(0, 0))
        return vect;
    float vectlen = sqrt(pow(vect.x, 2) + pow(vect.y, 2));
    return sf::operator/=(vect, vectlen);
}

int main() {


    //init
    sf::RenderWindow window(sf::VideoMode(900, 900), "Jet Fighters");
    sf::Event event;
    sf::Clock clock;
    float dt;
    sf::Texture characterTexture;
    characterTexture.loadFromFile("player.png");
    sf::Sprite character(characterTexture);
    character.setOrigin(sf::Vector2f(character.getTextureRect().width / 2, character.getTextureRect().height / 2));
    
    character.setPosition(sf::Vector2f(450, 850));

    while (window.isOpen()) {
        dt = (float)clock.restart().asMicroseconds()/1000000; //deltaTime
        sf::Vector2f currentPos = character.getPosition();
        sf::Vector2f velocity = sf::Vector2f(0, 0);
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }
        //przechodzenie na przeciwn¹ czêœæ ekranu
        if (currentPos.x > 945)
            currentPos.x = -20;
        if (currentPos.x < -45)
            currentPos.x = 920;
        //kierunek ruchu
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
            velocity.x -= 1;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
            velocity.x += 1;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
            if (currentPos.y >= 60)
                velocity.y -= 1;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
            if (currentPos.y <= 840)
                velocity.y += 1;
        }
        
        //normalizacja wektora prêdkoœci (d³ugoœæ 1)
        velocity = normalizeVector(velocity);
        //ruch
        character.setPosition(currentPos + sf::operator*(sf::operator*(velocity, SPEED),dt));
        std::cout << dt <<std::endl;
        window.clear(sf::Color::White);
        window.draw(character);
        window.display();
    }
    return 0;
}
