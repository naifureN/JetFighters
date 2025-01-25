#include <SFML/Graphics.hpp>
#include <SFML/Window/Cursor.hpp>
#include <iostream>
#include <random>
#include <string>
#include <iomanip>
#include <sstream>
#include <fstream>
using namespace sf;

const float SPEED = 600;
const float ENEMY_SPEED = 300;
const float BULLET_SPEED = 900;
const float SHOOT_SPEED = 0.5;
const float BCG_SPEED = 200;
const float INVULNERABLE_TIME = 3;
char state = 'm'; // m = menu, p = playing, l = lost, c = controls
bool end = false;
bool clickable = false;
int score;
float modifier;

Sprite playerSprite;
//inicjalizacja obiektów tekstu
Font font;
Text scoreText;
Text hscoreText;
Text hpText;
Text modifierText;
Text lostText;
Text controlsText;

//Tworzenie wektora o długości 1
Vector2f normalizeVector(Vector2f vect) {
    if (vect == Vector2f(0, 0))
        return vect;
    float vectlen = sqrt(pow(vect.x, 2) + pow(vect.y, 2));
    return operator/=(vect, vectlen);
}

//Struktura amunicji
struct Bullet {
    Vector2f size = Vector2f(40, 70);
    Vector2f origin = Vector2f(20, 35);
    Vector2f position = Vector2f(450, 800);
    Vector2f direction = Vector2f(0, 0);
    int caster;
    bool active = false;
    void move(float deltaTime) {
        if (caster == 0) {
            direction.y = -1;
        }
        float speed = BULLET_SPEED * (modifier * 1.5);
        operator+=(position, operator*(operator*(direction, speed), deltaTime));
    }
};

Bullet bullets[128];
Sprite bulletSprites[128];

//Struktura przeciwnika
struct Enemy {
    Vector2f size = Vector2f(61, 94);
    Vector2f origin = Vector2f(61/2, 94/2);
    Vector2f position = Vector2f(100 + rand() % 600, -100);
    Vector2f direction = Vector2f(-10 + rand() % 20, 1 + rand() % 9);
    Clock directionTimer;
    const int caster = 1;
    bool active = false;
    void move(float deltaTime) {
        direction = normalizeVector(direction);
        if (position.x > 945)
            position.x = -20;
        if (position.x < -45)
            position.x = 920;
        float speed = ENEMY_SPEED * modifier;
        operator+=(position, operator*(operator*(direction, speed), deltaTime));
    }
    void activate() {
        active = true;
        directionTimer.restart();
        position = Vector2f(100 + rand() % 600, -100);
    }
    void change_direction() {
        direction.x = -10 + rand() % 20;
        direction.y = 1 + rand() % 9;
    }
};

Enemy enemies[128];
Sprite enemySprites[128];

//Struktura gracza
Clock hit_timer;
Clock pulse_timer;
struct Player {
    const int caster = 0;
    int hp = 5;
    Vector2f size = Vector2f(100, 142);
    Vector2f origin = Vector2f(50, 71);
    Vector2f position = Vector2f(450, 800);
    Vector2f direction = Vector2f(0, 0);
    bool invulnerable = false;
    void reset() {
        position = Vector2f(450, 800);
        hp = 5;
        invulnerable = false;
        score = 0;
        modifier = 1;
    }

    void move(float deltaTime) {
        float speed = SPEED * (modifier * 1.2);
        operator+=(position, operator*(operator*(direction, speed), deltaTime));
    }
    void hit() {
        hit_timer.restart();
        hp -= 1;
        Color playerColor = playerSprite.getColor();
        playerSprite.setColor(Color(playerColor.r, playerColor.g, playerColor.b, 128));
        invulnerable = true;
    }
    void pulsate() {
        if ((float)pulse_timer.getElapsedTime().asMicroseconds() / 1000000 >= 0.1) {
            Color playerColor = playerSprite.getColor();
            if (playerColor.a == 255)
                playerSprite.setColor(Color(playerColor.r, playerColor.g, playerColor.b, 128));
            else
                playerSprite.setColor(Color(playerColor.r, playerColor.g, playerColor.b, 255));
            pulse_timer.restart();
        }
    }
    void addScore() {
        score += 100 * modifier;
    }
};
Player player;
Texture bulletTexture;
Texture enemyTexture;

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
            FloatRect enemyBounds(
                enemies[i].position.x - enemies[i].origin.x,
                enemies[i].position.y - enemies[i].origin.y,
                enemies[i].size.x,
                enemies[i].size.y
            );
            for (int j = 0; j < 128; j++) {
                if (bullets[j].active) {
                    FloatRect bulletBounds(
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
        for (int i = 0; i < 128; i++) {
            if (enemies[i].active) {
                FloatRect enemyBounds(
                    enemies[i].position.x - 15,
                    enemies[i].position.y - 40,
                    30,
                    80
                );
                FloatRect playerBounds(
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
            }
        }
    }
}

//aktualizacja zmiennej od której zależy poziom trudności
void changeModifier() {
    float newnum = (float(3 + rand() % 9)) / 100;
    modifier += newnum;
}

//inicjalizacja sprite'ów przycisków oraz niektórych timerów
Sprite exitBtn;
Sprite startBtn;
Sprite restartBtn;
Sprite menuBtn;
Sprite controlsBtn;
Clock tmr;
Clock modifierClock;

//sprawdzanie czy przyciski są naciśnięte
void checkButtons(Vector2i mousePos) {
    FloatRect exitBtnBounds(
        exitBtn.getPosition().x,
        exitBtn.getPosition().y,
        300,
        100
    );
    FloatRect startBtnBounds(
        startBtn.getPosition().x,
        startBtn.getPosition().y,
        300,
        100
    );
    FloatRect menuBtnBounds(
        menuBtn.getPosition().x,
        menuBtn.getPosition().y,
        300,
        100
    );
    FloatRect restartBtnBounds(
        restartBtn.getPosition().x,
        restartBtn.getPosition().y,
        300,
        100
    );
    FloatRect controlsBtnBounds(
        controlsBtn.getPosition().x,
        controlsBtn.getPosition().y,
        300,
        100
    );
    //przycisk musi zostać naciśnięty później niż 0.2 sekundy po poprzednim przycisku
    if (tmr.getElapsedTime().asMicroseconds() >= 200000) {
        if (state == 'm') {
            if (exitBtnBounds.contains(mousePos.x, mousePos.y)) {
                clickable = true;
                if (Mouse::isButtonPressed(Mouse::Left)) {
                    end = true;
                    tmr.restart();
                }
            }
            else if (startBtnBounds.contains(mousePos.x, mousePos.y)) {
                clickable = true;
                if (Mouse::isButtonPressed(Mouse::Left)) {
                    state = 'p';
                    player.reset();
                    modifierClock.restart();
                    tmr.restart();
                }
            }
            else if (controlsBtnBounds.contains(mousePos.x, mousePos.y)) {
                clickable = true;
                if (Mouse::isButtonPressed(Mouse::Left)) {
                    state = 'c';
                    tmr.restart();
                }
            }
            else {
                clickable = false;
            }
        }
        else if (state == 'c') {
            if (menuBtnBounds.contains(mousePos.x, mousePos.y)) {
                clickable = true;
                if (Mouse::isButtonPressed(Mouse::Left)) {
                    state = 'm';
                    tmr.restart();
                }
            }
        }
        else if (state == 'l') {
            if (menuBtnBounds.contains(mousePos.x, mousePos.y)) {
                clickable = true;
                if (Mouse::isButtonPressed(Mouse::Left)) {
                    state = 'm';
                    tmr.restart();
                }
            }
            else if (restartBtnBounds.contains(mousePos.x, mousePos.y)) {
                clickable = true;
                if (Mouse::isButtonPressed(Mouse::Left)) {
                    state = 'p';
                    player.reset();
                    modifierClock.restart();
                    tmr.restart();
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
}

//dezaktywacja przeciwników oraz amunicji po grze 
void deactivate_game() {
    for (int i = 0; i < 128; i++) {
        bullets[i].active = false;
        enemies[i].active = false;
    }
}

//odczytywanie najwyższego wyniku z pliku
int read_high_score() {
    std::string high_score_str;
    std::string hscore_file_name = "high_score.txt";
    std::ifstream hscore_file(hscore_file_name);
    std::getline(hscore_file, high_score_str);
    hscore_file.close();
    return std::stoi(high_score_str);
}

//zapisywanie najwyższego wyniku do pliku
void save_high_score(int scr, int hscr) {
    if (scr > hscr) {
        hscr = scr;
        std::string hscore_file_name = "high_score.txt";
        std::ofstream hscore_file(hscore_file_name);
        hscore_file << hscr;
        hscore_file.close();
    }
}

int main() {
    //init
    int high_score = read_high_score();
    srand(time(NULL));

    //Tworzenie wszystkich tekstów
    font.loadFromFile("Symtext.ttf");
    scoreText.setFont(font);
    scoreText.setCharacterSize(48);
    scoreText.setFillColor(Color::Black);
    scoreText.setPosition(Vector2f(100, 100));
    hscoreText.setFont(font);
    hscoreText.setCharacterSize(48);
    hscoreText.setFillColor(Color::Black);
    hscoreText.setPosition(Vector2f(100, 100));
    hpText.setFont(font);
    hpText.setCharacterSize(48);
    hpText.setFillColor(Color::Black);
    hpText.setPosition(Vector2f(15, 0));
    modifierText.setFont(font);
    modifierText.setCharacterSize(24);
    modifierText.setFillColor(Color::Black);
    modifierText.setPosition(Vector2f(15, 0));
    lostText.setFont(font);
    lostText.setCharacterSize(92);
    lostText.setFillColor(Color::Black);
    lostText.setPosition(Vector2f(15, 0));
    lostText.setString("You lost!");
    controlsText.setFont(font);
    controlsText.setCharacterSize(32);
    controlsText.setFillColor(Color::Black);
    controlsText.setPosition(Vector2f(100, 100));

    RenderWindow window(VideoMode(900, 900), "Jet Fighters");
    Event event;

    //tworzenie timerów
    Clock clock;
    Clock shootTimer;
    Clock spawner;
    float dt;

    //wczytanie różnych kursorów
    Cursor cursor_normal;
    cursor_normal.loadFromSystem(Cursor::Arrow);
    Cursor cursor_clickable;
    cursor_clickable.loadFromSystem(Cursor::Hand);
    window.setMouseCursor(cursor_normal);
    
    //tworzenie tła
    Texture backgroundTexture;
    backgroundTexture.loadFromFile("bckgrnd.png");
    Sprite back1;
    back1.setTexture(backgroundTexture);
    Sprite back2;
    back2.setTexture(backgroundTexture);
    back2.setPosition(Vector2f(back1.getPosition().x, back1.getPosition().y - 1195));

    //tworzenie logo do menu głównego oraz przycisków
    Texture logoTexture;
    logoTexture.loadFromFile("logo.png");
    Sprite logo;
    logo.setTexture(logoTexture);
    logo.setOrigin(Vector2f(400, 0));
    logo.setPosition(Vector2f(450, 100));

    Texture startbtnTexture;
    startbtnTexture.loadFromFile("btnstart.png");
    startBtn.setTexture(startbtnTexture);
    startBtn.setPosition(Vector2f(300, 400));

    Texture exitbtnTexture;
    exitbtnTexture.loadFromFile("btnexit.png");
    exitBtn.setTexture(exitbtnTexture);
    exitBtn.setPosition(Vector2f(300, 700));

    Texture restartbtnTexture;
    restartbtnTexture.loadFromFile("btnrestart.png");
    restartBtn.setTexture(restartbtnTexture);
    restartBtn.setPosition(Vector2f(300, 550));

    Texture menubtnTexture;
    menubtnTexture.loadFromFile("btnmenu.png");
    menuBtn.setTexture(menubtnTexture);

    Texture controlsbtnTexture;
    controlsbtnTexture.loadFromFile("btncontrols.png");
    controlsBtn.setTexture(controlsbtnTexture);
    controlsBtn.setPosition(Vector2f(300, 550));

    Texture playerTexture;
    playerTexture.loadFromFile("player.png");
    playerSprite.setTexture(playerTexture);
    playerSprite.setOrigin(player.origin);
    playerSprite.setPosition(player.position);

    //wczytanie tekstur amunicji oraz przeciwnika
    bulletTexture.loadFromFile("bullet.png");
    enemyTexture.loadFromFile("enemy.png");
    
    //tworzenie przeciwników oraz amunicji
    for (int i = 0; i < 128; i++){
        bulletSprites[i].setTexture(bulletTexture);
        bulletSprites[i].setOrigin(bullets[i].origin);

        enemySprites[i].setTexture(enemyTexture);
        enemySprites[i].setOrigin(enemies[i].origin);
        enemies[i].directionTimer.restart();
    }

    //tworzenie stringów wykorzystywanych później w tekstach
    std::string scoreString;
    std::string hscoreString;
    std::string modifierString;
    std::string hpString;

    //koniec initu


    while (window.isOpen()) {
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed)
                window.close();
        }
        //zwiększenie trudności po 10 sekundach
        if (modifierClock.getElapsedTime().asSeconds() >= 10) {
            changeModifier();
            modifierClock.restart();
        }

        dt = (float)clock.restart().asMicroseconds() / 1000000; //deltaTime
        checkButtons(Mouse::getPosition(window));

        //zmiana kursora gdy można na coś nacisnąć
        if (clickable == false) {
            window.setMouseCursor(cursor_normal);
        }
        else {
            window.setMouseCursor(cursor_clickable);
        }

        //główna gra
        if (state == 'p') {
            //aktualizacja tekstu w HUDzie
            scoreString = "Score: " + std::to_string(score);
            scoreText.setString(scoreString);
            std::ostringstream stream;
            stream << std::fixed << std::setprecision(2) << modifier;
            modifierText.setString("Modifier: " + stream.str());
            hpString = "HP: " + std::to_string(player.hp);
            hpText.setString(hpString);
            player.direction = Vector2f(0, 0);

            //koniec gry po śmierci
            if (player.hp <= 0) {
                state = 'l';
                save_high_score(score, high_score);
                high_score = read_high_score();
                deactivate_game();
            }

            //wyrównywanie tekstu
            scoreText.setPosition(Vector2f(window.getSize().x-scoreText.getLocalBounds().width-15,0));
            modifierText.setPosition(Vector2f((window.getSize().x/2) - (modifierText.getLocalBounds().width / 2), window.getSize().y-(15+32)));
            
            //Poruszanie tła
            Vector2f bcg1pos = back1.getPosition();
            Vector2f bcg2pos = back2.getPosition();
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

            //zmiana przezroczystości oraz chwilowa nietykalność po uderzeniu
            if ((float)hit_timer.getElapsedTime().asMicroseconds() / 1000000 >= INVULNERABLE_TIME/modifier) {
                Color playerColor = playerSprite.getColor();
                playerSprite.setColor(Color(playerColor.r, playerColor.g, playerColor.b, 255));
                player.invulnerable = false;
            }

            //przechodzenie na przeciwną część ekranu
            if (player.position.x > 950)
                player.position.x = -49;
            if (player.position.x < -50)
                player.position.x = 949;
            
            //kierunek ruchu
            if (Keyboard::isKeyPressed(Keyboard::A) or Keyboard::isKeyPressed(Keyboard::Left)) {
                player.direction.x -= 1;
            }
            if (Keyboard::isKeyPressed(Keyboard::D) or Keyboard::isKeyPressed(Keyboard::Right)) {
                player.direction.x += 1;
            }
            if (Keyboard::isKeyPressed(Keyboard::W) or Keyboard::isKeyPressed(Keyboard::Up)) {
                if (player.position.y >= 71+5)
                    player.direction.y -= 1;
            }
            if (Keyboard::isKeyPressed(Keyboard::S) or Keyboard::isKeyPressed(Keyboard::Down)) {
                if (player.position.y <= 900-71-5)
                    player.direction.y += 1;
            }
            //strzelanie
            if (Keyboard::isKeyPressed(Keyboard::Space)) {
                if ((float)shootTimer.getElapsedTime().asMicroseconds() / 1000000 > (SHOOT_SPEED / modifier)) {
                    shoot(player.caster);
                    shootTimer.restart();
                }
            }
            //Tworzenie przeciwników
            if ((float)spawner.getElapsedTime().asMicroseconds() / 1000000 >= 1/(modifier*1.2)) {
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
                    if (enemies[i].directionTimer.getElapsedTime().asSeconds() >= (5 - rand()%3)/modifier) {
                        enemies[i].change_direction();
                        enemies[i].directionTimer.restart();
                    }
                    enemies[i].move(dt);
                    enemySprites[i].setPosition(enemies[i].position);
                }
            }
            updateEntities();
            checkCollisions();

            if (player.invulnerable == true)
                player.pulsate();

            //rysowanie wszystkiego co aktywne na ekran
            window.clear(Color::White);
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
            window.draw(modifierText);
            window.draw(playerSprite);
            window.draw(scoreText);
            window.draw(hpText);

        }
        //Main menu
        else if (state == 'm') {
            modifier = 1.00;

            //wyłączenie gry
            if (end == true)
                window.close();

            //ruch tła
            Vector2f bcg1pos = back1.getPosition();
            Vector2f bcg2pos = back2.getPosition();
            bcg1pos.y += BCG_SPEED * modifier * 0.1 * dt;
            bcg2pos.y += BCG_SPEED * modifier * 0.1 * dt;
            if (bcg1pos.y >= 900) {
                bcg1pos.y = bcg2pos.y - 1195;
            }
            else if (bcg2pos.y >= 900) {
                bcg2pos.y = bcg1pos.y - 1195;
            }

            //rysowanie na ekran
            back1.setPosition(bcg1pos);
            back2.setPosition(bcg2pos);
            window.clear();
            window.draw(back1);
            window.draw(back2);
            window.draw(startBtn);
            window.draw(exitBtn);
            window.draw(controlsBtn);
            window.draw(logo);
        }
        //Lose screen
        else if (state == 'l') {
            //aktualizacja tekstów oraz ustawienie przycisków
            scoreString = "Score: " + std::to_string(score);
            hscoreString = "High score: " + std::to_string(high_score);
            scoreText.setString(scoreString);
            scoreText.setPosition(Vector2f(window.getSize().x / 2 - scoreText.getLocalBounds().width / 2, 200));
            hscoreText.setString(hscoreString);
            hscoreText.setPosition(Vector2f(window.getSize().x / 2 - hscoreText.getLocalBounds().width / 2, 300));
            lostText.setPosition(Vector2f(window.getSize().x / 2 - lostText.getLocalBounds().width / 2, 50));
            menuBtn.setPosition(Vector2f(300, 700));

            //poruszanie tłem
            Vector2f bcg1pos = back1.getPosition();
            Vector2f bcg2pos = back2.getPosition();
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

            //rysowanie na ekran
            window.clear();
            window.draw(back1);
            window.draw(back2);
            window.draw(restartBtn);
            window.draw(menuBtn);
            window.draw(scoreText);
            window.draw(hscoreText);
            window.draw(lostText);
        }

        //ekran sterowania
        else if (state == 'c') {
            menuBtn.setPosition(Vector2f(300, 700));

            //poruszanie tła
            Vector2f bcg1pos = back1.getPosition();
            Vector2f bcg2pos = back2.getPosition();
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

            //aktualizacja tekstu
            controlsText.setString("WSAD/STRZALKI/ARROWS - MOVE/RUCH\nSPACJA/SPACE - SHOOT/STRZAL\n\n\nYOU NEED TO SHOOT DOWN ENEMIES\nTHE GOAL IS TO GET HIGHEST SCORE\n\nMUSISZ STRZELAC WE WROGÓW\nCELEM JEST ZDOBYCIE NAJWYZSZEGO WYNIKU");
            controlsText.setPosition(Vector2f(window.getSize().x / 2 - controlsText.getLocalBounds().width / 2, 150));
            
            //rysowanie na ekran
            window.clear();
            window.draw(back1);
            window.draw(back2);
            window.draw(menuBtn);
            window.draw(controlsText);
        }
        //wyświetlanie okna
        window.display();
    }
    return 0;
}
