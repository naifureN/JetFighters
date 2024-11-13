#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <random>

const float SPEED = 800;
const float ENEMY_SPEED = 300;
const float BULLET_SPEED = 900;
const float SHOOT_SPEED = 0.2;

//Tworzenie wektora o d³ugoœci 1
sf::Vector2f normalizeVector(sf::Vector2f vect) {
    if (vect == sf::Vector2f(0, 0))
        return vect;
    float vectlen = sqrt(pow(vect.x, 2) + pow(vect.y, 2));
    return sf::operator/=(vect, vectlen);
}

//Struktura amunicji
struct Bullet {
    sf::Vector2f size = sf::Vector2f(40, 40);
    sf::Vector2f origin = sf::Vector2f(20, 20);
    sf::Vector2f position = sf::Vector2f(450, 800);
    sf::Vector2f direction = sf::Vector2f(0, 0);
    int caster;
    bool active = false;
    void move(float deltaTime) {
        if (caster == 0) {
            direction.y = -1;
        }
        sf::operator+=(position, sf::operator*(sf::operator*(direction, BULLET_SPEED), deltaTime));
    }
};

Bullet bullets[128];
sf::Sprite bulletSprites[128];

//Struktura przeciwnika
struct Enemy {
    sf::Vector2f size = sf::Vector2f(60, 60);
    sf::Vector2f origin = sf::Vector2f(30, 30);
    sf::Vector2f position = sf::Vector2f(0 + rand() % 300, -100);
    sf::Vector2f direction = sf::Vector2f(0, 0);
    const int caster = 1;
    bool active = false;
    int pattern = 1;
    void move(float deltaTime) {
        if (pattern == 1) {
            direction = normalizeVector(sf::Vector2f(4, 1));
        }
        else {
            direction = normalizeVector(sf::Vector2f(-4, 1));
        }
        if (position.x > 945)
            position.x = -20;
        if (position.x < -45)
            position.x = 920;

        sf::operator+=(position, sf::operator*(sf::operator*(direction, ENEMY_SPEED), deltaTime));
    }
    void activate() {
        active = true;
        position = sf::Vector2f(0 + rand() % 300, -100);
    }
};

Enemy enemies[128];
sf::Sprite enemySprites[128];

//Struktura gracza
struct Player {
    const int caster = 0;
    sf::Vector2f size = sf::Vector2f(100, 100);
    sf::Vector2f origin = sf::Vector2f(50, 50);
    sf::Vector2f position = sf::Vector2f(450, 800);
    sf::Vector2f direction = sf::Vector2f(0, 0);

    void move(float deltaTime) {
        sf::operator+=(position, sf::operator*(sf::operator*(direction, SPEED), deltaTime));
    }
};
Player player;
sf::Texture bulletTexture;
sf::Texture enemyTexture;

//Funkcja strzelania
void shoot(int caster) {
    int blt = 0;
    for (int i = 0; i < 128; i++) {
        if (bullets[i].active == false) {
            bullets[i].active = true;
            blt = i;
            break;
        }
    }
    if (caster == 0) {
        bullets[blt].position = player.position;
        bullets[blt].position.y -= 20;
    }
}

//Deaktywacja przeciwników i amunicji poza ekranem
void updateEntities() {
    for (int i = 0; i < 128; i++) {
        if (bullets[i].position.y < -20)
            bullets[i].active = false;
        if (enemies[i].position.y > 950)
            enemies[i].active = false;
    }
}

//Wykrywanie kolizji
void checkCollisions() {
    for (int i = 0; i < 128; i++) {
        if (enemies[i].active) {
            sf::FloatRect enemyBounds(
                enemies[i].position.x - enemies[i].origin.x,
                enemies[i].position.y - enemies[i].origin.y,
                enemies[i].size.x,
                enemies[i].size.y
            );

            for (int j = 0; j < 128; j++) {
                if (bullets[j].active) {
                    sf::FloatRect bulletBounds(
                        bullets[j].position.x - bullets[j].origin.x,
                        bullets[j].position.y - bullets[j].origin.y,
                        bullets[j].size.x,
                        bullets[j].size.y
                    );

                    if (enemyBounds.intersects(bulletBounds)) {
                        bullets[j].active = false;
                        enemies[i].active = false;
                        break;
                    }
                }
            }
        }
    }
}
int main() {
    //init
    srand(time(NULL));
    sf::RenderWindow window(sf::VideoMode(900, 900), "Jet Fighters");
    sf::Event event;
    sf::Clock clock;
    sf::Clock shootTimer;
    sf::Clock spawner;
    float dt;

    sf::Texture playerTexture;
    playerTexture.loadFromFile("player.png");
    sf::Sprite playerSprite(playerTexture);
    playerSprite.setOrigin(player.origin);
    playerSprite.setPosition(player.position);

    bulletTexture.loadFromFile("bullet.png");
    enemyTexture.loadFromFile("enemy.png");

    for (int i = 0; i < 128; i++){
        bulletSprites[i].setTexture(bulletTexture);
        bulletSprites[i].setOrigin(bullets[i].origin);

        enemySprites[i].setTexture(enemyTexture);
        enemySprites[i].setOrigin(enemies[i].origin);
    }
    //koniec initu


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
        //strzelanie
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
            if ((float)shootTimer.getElapsedTime().asMicroseconds() / 1000000 > SHOOT_SPEED) {
                shoot(player.caster);
                shootTimer.restart();
            }
        }
        //Tworzenie przeciwników
        if ((float)spawner.getElapsedTime().asMicroseconds() / 1000000 > 1) {
            for (int i = 0; i < 128; i++) {
                if (false == enemies[i].active) {
                    enemies[i].activate();
                    spawner.restart();
                    break;
                }
            }
        }
        //ruch gracza
        player.direction = normalizeVector(player.direction);
        player.move(dt);
        playerSprite.setPosition(player.position);
        //ruch amunicji
        for (int i = 0; i < 128; i++) {
            if (bullets[i].active == true) {
                bullets[i].move(dt);
                bulletSprites[i].setPosition(bullets[i].position);
            }
        }
        //ruch przeciwników
        for (int i = 0; i < 128; i++) {
            if (enemies[i].active == true) {
                enemies[i].move(dt);
                enemySprites[i].setPosition(enemies[i].position);
            }
        }
        updateEntities();
        checkCollisions();
        //rysowanie wszystkiego co aktywne na ekran
        window.clear(sf::Color::White);
        for (int i = 0; i < 128; i++) {
            if (true == enemies[i].active)
                window.draw(enemySprites[i]);
        }
        for (int i = 0; i < 128; i++) {
            if (true == bullets[i].active)
                window.draw(bulletSprites[i]);
        }
        window.draw(playerSprite);
        window.display();
    }
    return 0;
}
