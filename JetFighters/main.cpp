#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Window/Cursor.hpp>
#include <iostream>
#include <random>
#include <string>

const float SPEED = 800;
const float ENEMY_SPEED = 300;
const float BULLET_SPEED = 900;
const float SHOOT_SPEED = 0.35;
const float BCG_SPEED = 200;
char state = 'm'; // m = menu, p = playing
bool end = false;
bool clickable = false;
int score;
float modifier;

sf::Sprite playerSprite;
sf::Font font;
sf::Text scoreText;
sf::Text hpText;

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
    sf::Vector2f size = sf::Vector2f(40, 70);
    sf::Vector2f origin = sf::Vector2f(40, 40);
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
sf::Clock hit_timer;
sf::Clock pulse_timer;
struct Player {
    const int caster = 0;
    int hp = 5;
    sf::Vector2f size = sf::Vector2f(70, 70);
    sf::Vector2f origin = sf::Vector2f(50, 50);
    sf::Vector2f position = sf::Vector2f(450, 800);
    sf::Vector2f direction = sf::Vector2f(0, 0);
    bool invulnerable = false;
    void reset() {
        position = sf::Vector2f(450, 800);
        hp = 5;
        invulnerable = false;
        score = 0;
        modifier = 1;
    }

    void move(float deltaTime) {
        sf::operator+=(position, sf::operator*(sf::operator*(direction, SPEED), deltaTime));
    }
    void hit() {
        hit_timer.restart();
        hp -= 1;
        sf::Color playerColor = playerSprite.getColor();
        playerSprite.setColor(sf::Color(playerColor.r, playerColor.g, playerColor.b, 128));
        invulnerable = true;
    }
    void pulsate() {
        if ((float)pulse_timer.getElapsedTime().asMicroseconds() / 1000000 >= 0.1) {
            sf::Color playerColor = playerSprite.getColor();
            if (playerColor.a == 255)
                playerSprite.setColor(sf::Color(playerColor.r, playerColor.g, playerColor.b, 128));
            else
                playerSprite.setColor(sf::Color(playerColor.r, playerColor.g, playerColor.b, 255));
            pulse_timer.restart();
        }
    }
    void addScore() {
        score += 100 * modifier;
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
        bullets[blt].position.y -= 60;
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
                enemies[i].position.x - enemies[i].origin.x/2,
                enemies[i].position.y - enemies[i].origin.y,
                enemies[i].size.x,
                enemies[i].size.y
            );
            sf::FloatRect playerBounds(
                player.position.x - player.origin.x,
                player.position.y - player.origin.y,
                player.size.x,
                player.size.y
            );
            if (player.invulnerable == false) {
                if (enemyBounds.intersects(playerBounds)) {
                    player.hit();
                    enemies[i].active = false;
                    break;
                }
            }
            for (int j = 0; j < 128; j++) {
                if (bullets[j].active) {
                    sf::FloatRect bulletBounds(
                        bullets[j].position.x - bullets[j].origin.x,
                        bullets[j].position.y - bullets[j].origin.y,
                        bullets[j].size.x,
                        bullets[j].size.y
                    );

                    if (enemyBounds.intersects(bulletBounds)) {
                        if (bullets[j].caster == player.caster) {
                            player.addScore();
                            bullets[j].active = false;
                            enemies[i].active = false;
                        }

                        break;
                    }
                }
            }
        }
    }
}
void changeModifier() {
    modifier += 0.1;
}

sf::Sprite exitBtn;
sf::Sprite startBtn;
sf::Clock modifierClock;
void checkButtons(sf::Vector2i mousePos) {
    
    sf::FloatRect exitBtnBounds(
        exitBtn.getPosition().x,
        exitBtn.getPosition().y,
        300,
        100
    );
    sf::FloatRect startBtnBounds(
        startBtn.getPosition().x,
        startBtn.getPosition().y,
        300,
        100
    );
    if (state == 'm') {
        if (exitBtnBounds.contains(mousePos.x, mousePos.y)) {
            clickable = true;
            if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
                end = true;
            }
        }
        else if (startBtnBounds.contains(mousePos.x, mousePos.y)) {
            
            clickable = true;
            if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
                state = 'p';
                player.reset();
                modifierClock.restart();
            }
        }
        else {
            clickable = false;
        }
    }
    else {
        clickable = false;
    }
}

void deactivate_game() {
    for (int i = 0; i < 128; i++) {
        bullets[i].active = false;
        enemies[i].active = false;
    }
}

int main() {
    //init
    srand(time(NULL));
    font.loadFromFile("Symtext.ttf");
    scoreText.setFont(font);
    scoreText.setCharacterSize(48);
    scoreText.setFillColor(sf::Color::Black);
    scoreText.setPosition(sf::Vector2f(100, 100));
    hpText.setFont(font);
    hpText.setCharacterSize(48);
    hpText.setFillColor(sf::Color::Black);
    hpText.setPosition(sf::Vector2f(15, 0));
    sf::RenderWindow window(sf::VideoMode(900, 900), "Jet Fighters");
    sf::Event event;
    sf::Clock clock;
    sf::Clock shootTimer;
    sf::Clock spawner;
    float dt;

    sf::Cursor cursor_normal;
    cursor_normal.loadFromSystem(sf::Cursor::Arrow);
    sf::Cursor cursor_clickable;
    cursor_clickable.loadFromSystem(sf::Cursor::Hand);
    window.setMouseCursor(cursor_normal);
    
    sf::Texture backgroundTexture;
    backgroundTexture.loadFromFile("bckgrnd.png");
    sf::Sprite back1;
    back1.setTexture(backgroundTexture);
    sf::Sprite back2;
    back2.setTexture(backgroundTexture);
    back2.setPosition(sf::Vector2f(back1.getPosition().x, back1.getPosition().y - 1195));

    sf::Texture startbtnTexture;
    startbtnTexture.loadFromFile("btnstart.png");
    startBtn.setTexture(startbtnTexture);
    sf::Texture exitbtnTexture;
    exitbtnTexture.loadFromFile("btnexit.png");
    exitBtn.setTexture(exitbtnTexture);
    startBtn.setPosition(sf::Vector2f(300, 300));
    exitBtn.setPosition(sf::Vector2f(300, 500));


    sf::Texture playerTexture;
    playerTexture.loadFromFile("player.png");
    playerSprite.setTexture(playerTexture);
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
    std::string scoreString;
    std::string hpString;
    //koniec initu


    while (window.isOpen()) {
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }
        if (modifierClock.getElapsedTime().asSeconds() >= 10) {
            changeModifier();
            modifierClock.restart();
        }
        std::cout << modifier << std::endl;
        dt = (float)clock.restart().asMicroseconds() / 1000000; //deltaTime
        checkButtons(sf::Mouse::getPosition(window));
        if (clickable == false) {
            window.setMouseCursor(cursor_normal);
        }
        
        else {
            window.setMouseCursor(cursor_clickable);
        }
        if (state == 'p') {
            scoreString = "Score: " + std::to_string(score);
            scoreText.setString(scoreString);
            hpString = "HP: " + std::to_string(player.hp);
            hpText.setString(hpString);
            player.direction = sf::Vector2f(0, 0);

            if (player.hp <= 0) {
                state = 'm';
                deactivate_game();
            }
            scoreText.setPosition(sf::Vector2f(window.getSize().x-scoreText.getLocalBounds().width-15,0));
            //Poruszanie t³a
            sf::Vector2f bcg1pos = back1.getPosition();
            sf::Vector2f bcg2pos = back2.getPosition();
            bcg1pos.y += BCG_SPEED * modifier * dt;
            bcg2pos.y += BCG_SPEED * modifier * dt;
            if (bcg1pos.y >= 900) {
                bcg1pos.y = bcg2pos.y - 1195;
            }
            else if (bcg2pos.y >= 900) {
                bcg2pos.y = bcg1pos.y - 1195;
            }
            back1.setPosition(bcg1pos);
            back2.setPosition(bcg2pos);

            //zmiana koloru po uderzeniu
            if ((float)hit_timer.getElapsedTime().asMicroseconds() / 1000000 >= 1) {
                sf::Color playerColor = playerSprite.getColor();
                playerSprite.setColor(sf::Color(playerColor.r, playerColor.g, playerColor.b, 255));
                player.invulnerable = false;
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
            if (player.invulnerable == true)
                player.pulsate();
            //rysowanie wszystkiego co aktywne na ekran
            window.clear(sf::Color::White);
            window.draw(back1);
            window.draw(back2);
            for (int i = 0; i < 128; i++) {
                if (true == enemies[i].active)
                    window.draw(enemySprites[i]);
            }
            for (int i = 0; i < 128; i++) {
                if (true == bullets[i].active)
                    window.draw(bulletSprites[i]);
            }
            window.draw(playerSprite);
            window.draw(scoreText);
            window.draw(hpText);

        }
        //Main menu
        else if (state == 'm') {
            if (end == true)
                window.close();
            sf::Vector2f bcg1pos = back1.getPosition();
            sf::Vector2f bcg2pos = back2.getPosition();
            bcg1pos.y += BCG_SPEED * modifier * 0.1 * dt;
            bcg2pos.y += BCG_SPEED * modifier * 0.1 * dt;
            if (bcg1pos.y >= 900) {
                bcg1pos.y = bcg2pos.y - 1195;
            }
            else if (bcg2pos.y >= 900) {
                bcg2pos.y = bcg1pos.y - 1195;
            }
            back1.setPosition(bcg1pos);
            back2.setPosition(bcg2pos);
            window.clear();
            window.draw(back1);
            window.draw(back2);
            window.draw(startBtn);
            window.draw(exitBtn);
        }
        window.display();
    }
    return 0;
}
