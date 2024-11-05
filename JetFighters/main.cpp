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

struct Player {
    sf::Vector2f size = sf::Vector2f(100, 100);
    sf::Vector2f origin = sf::Vector2f(50, 50);
    sf::Vector2f position = sf::Vector2f(450, 800);
    sf::Vector2f direction = sf::Vector2f(0, 0);
    

    void move(float deltaTime) {
        sf::operator+=(position, sf::operator*(sf::operator*(direction, SPEED), deltaTime));
    }
};

int main() {


    //init
    sf::RenderWindow window(sf::VideoMode(900, 900), "Jet Fighters");
    sf::Event event;
    sf::Clock clock;
    float dt;
    Player player;
    sf::Texture playerTexture;
    playerTexture.loadFromFile("player.png");
    sf::Sprite playerSprite(playerTexture);
    playerSprite.setOrigin(player.origin);
    playerSprite.setPosition(player.position);

    while (window.isOpen()) {
        dt = (float)clock.restart().asMicroseconds()/1000000; //deltaTime
        player.direction = sf::Vector2f(0, 0);

  
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }
        //przechodzenie na przeciwn¹ czêœæ ekranu
        if (player.position.x > 945)
            player.position.x = -20;
        if (player.position.x < -45)
            player.position.x = 920;
        //kierunek ruchu
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
            player.direction.x -= 1;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
            player.direction.x += 1;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
            if (player.position.y >= 60)
                player.direction.y -= 1;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
            if (player.position.y <= 840)
                player.direction.y += 1;
        }
            
        
        //normalizacja wektora prêdkoœci (d³ugoœæ 1)
        player.direction = normalizeVector(player.direction);
        //ruch
        player.move(dt);
        playerSprite.setPosition(player.position);
        //rysowanie wszystkiego na ekran
        window.clear(sf::Color::White);
        window.draw(playerSprite);
        window.display();
    }
    return 0;
}
