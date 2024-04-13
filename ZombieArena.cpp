#include <sstream>
#include <SFML/Graphics.hpp>
#include "Player.h"
#include "ZombieArena.h"
#include "TextureHolder.h"
#include "Bullet.h"
#include "Pickup.h"

using namespace sf;

int main()
{
    enum class State {
        PAUSED,
        LEVELING_UP,
        GAME_OVER,
        PLAYING
    };

    TextureHolder holder;

    State state = State::GAME_OVER;

    Vector2f resolution;
    resolution.x = VideoMode::getDesktopMode().width;
    resolution.y = VideoMode::getDesktopMode().height;
    RenderWindow window(
        VideoMode(resolution.x, resolution.y),
        "Zombie Arena",
        Style::Fullscreen
    );

    // Create a SFML view for the main action
    View mainView(sf::FloatRect(0, 0, resolution.x, resolution.y));

    Clock clock;
    Time gameTimeTotal;
    Vector2f mouseWorldPosition; // where is the mouse relative to world coordinates
    Vector2i mouseScreenPosition; // where is the mouse relative to screen coordinates

    Player player;

    IntRect arena;

    // Create background
    VertexArray background;
    Texture textureBackground = TextureHolder::GetTexture("graphics/background_sheet.png");

    // Zombie horde
    int numZombies;
    int numZombiesAlive;
    Zombie* zombies = nullptr;

    // 100 bullets
    Bullet bullets[100];
    int currentBullet = 0;
    int bulletsSpare = 24;
    int bulletsInClip = 6;
    int clipSize = 6;
    float fireRate = 1;
    Time lastPressed; // time fire button last pressed

    // hide mouse pointer and replace with crosshair
    window.setMouseCursorVisible(true);
    Sprite spriteCrosshair;
    spriteCrosshair.setTexture(TextureHolder::GetTexture("graphics/crosshair.png"));
    spriteCrosshair.setOrigin(25, 25);

    // create pickups
    Pickup healthPickup(1);
    Pickup ammoPickup(2);

    // about the game
    int score = 0;
    int highScore = 0;

    // Home/Game Over Screen
    Sprite spriteGameOver;
    Texture textureGameOver = TextureHolder::GetTexture("graphics/background.png");
    spriteGameOver.setTexture(textureGameOver);
    spriteGameOver.setPosition(0,0);

    // Create a view for the HUD
    View hudView(sf::FloatRect(0, 0, resolution.x, resolution.y));

    // Create a sprite for the ammo icon
    Sprite spriteAmmoIcon;
    Texture textureAmmoIcon = TextureHolder::GetTexture("graphics/ammo_icon.png");
    spriteAmmoIcon.setTexture(textureAmmoIcon);
    spriteAmmoIcon.setPosition(20, 980);

    // Load font
    Font font;
    font.loadFromFile("fonts/zombie-control/zombiecontrol.ttf");

    // Paused
    Text pausedText;
    pausedText.setFont(font);
    pausedText.setCharacterSize(155);
    pausedText.setFillColor(Color::White);
    pausedText.setPosition(400, 400);
    pausedText.setString("Press Enter to continue");

    // Game Over
    Text gameOverText;
    gameOverText.setFont(font);
    gameOverText.setCharacterSize(125);
    gameOverText.setFillColor(Color::White);
    gameOverText.setPosition(250, 850);
    gameOverText.setString("Press Enter to play");

    // Leveling up
    Text levelUpText;
    levelUpText.setFont(font);
    levelUpText.setCharacterSize(80);
    levelUpText.setFillColor(Color::White);
    levelUpText.setPosition(150, 250);
    std::stringstream levelUpStream;
    levelUpStream <<
        "1: Increased rate of fire" <<
        "\n2: Increased clip size (next reload)" <<
        "\n3: Increased max health" <<
        "\n4: Increased speed" <<
        "\n5: More and better health pickups" <<
        "\n6: More and better ammo pickups";
    levelUpText.setString(levelUpStream.str());

    // Ammo
    Text ammoText;
    ammoText.setFont(font);
    ammoText.setCharacterSize(55);
    ammoText.setFillColor(Color::White);
    ammoText.setPosition(200, 980);

    // Score
    Text scoreText;
    scoreText.setFont(font);
    ammoText.setCharacterSize(55);
    ammoText.setFillColor(Color::White);
    ammoText.setPosition(20, 0);

    // High Score
    Text highScoreText;
    highScoreText.setFont(font);
    highScoreText.setCharacterSize(55);
    highScoreText.setFillColor(Color::White);
    highScoreText.setPosition(1400, 0);
    std::stringstream highScoreStream;
    highScoreStream << "High Score: " << highScore;
    highScoreText.setString(highScoreStream.str());

    // Zombies remaining
    Text zombiesRemainingText;
    zombiesRemainingText.setFont(font);
    zombiesRemainingText.setCharacterSize(55);
    zombiesRemainingText.setFillColor(Color::White);
    zombiesRemainingText.setPosition(1500, 980);
    zombiesRemainingText.setString("Zombies: 100");

    // Wave number
    int wave = 0;
    Text waveNumberText;
    waveNumberText.setFont(font);
    waveNumberText.setCharacterSize(55);
    waveNumberText.setFillColor(Color::White);
    waveNumberText.setPosition(1250, 980);
    waveNumberText.setString("Wave: 0");

    // Health bar
    RectangleShape healthBar;
    healthBar.setFillColor(Color::Red);
    healthBar.setPosition(450, 980);

    // Debug HUD
    Text debugText;
    debugText.setFont(font);
    debugText.setCharacterSize(25);
    debugText.setFillColor(Color::White);
    debugText.setPosition(20, 220);
    std::ostringstream ss;

    // When was HUD last updated?
    int framesSinceLastHUDUpdate = 0;

    // How often in frames do we update the HUD?
    int fpsMeasurementFrameInterval = 1000;

    // Main game loop
    while (window.isOpen())
    {
        // handle input

        // handle events by polling
        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::KeyPressed)
            {
                // pause a game while playing
                if (event.key.code == Keyboard::Return && state == State::PLAYING)
                {
                    state = State::PAUSED;
                }

                // restart while paused
                if (event.key.code == Keyboard::Return && state == State::PAUSED)
                {
                    state = State::PLAYING;
                    clock.restart();
                }

                // start a new game
                if (event.key.code == Keyboard::Return && state == State::GAME_OVER)
                {
                    state = State::LEVELING_UP;
                }

                if (state == State::PLAYING)
                {
                    if (event.key.code == Keyboard::R)
                    {
                        if (bulletsSpare >= clipSize)
                        {
                            // plenty of bullets, reload
                            bulletsInClip = clipSize;
                            bulletsSpare -= clipSize;
                        }
                        else if (bulletsSpare > 0)
                        {
                            bulletsInClip = bulletsSpare;
                            bulletsSpare = 0;
                        }
                        else {
                            // tba
                        }
                    }
                }
            }
        } // End event polling

        // handle player quitting
        if (Keyboard::isKeyPressed(Keyboard::Escape))
        {
            window.close();
        }

        // Handle WASD while playing
        if (state == State::PLAYING)
        {
            if (Keyboard::isKeyPressed(Keyboard::W))
            {
                player.moveUp();
            }
            else
            {
                player.stopUp();
            }

            if (Keyboard::isKeyPressed(Keyboard::S))
            {
                player.moveDown();
            }
            else
            {
                player.stopDown();
            }

            if (Keyboard::isKeyPressed(Keyboard::A))
            {
                player.moveLeft();
            }
            else
            {
                player.stopLeft();
            }

            if (Keyboard::isKeyPressed(Keyboard::D))
            {
                player.moveRight();
            }
            else{
                player.stopRight();
            }

            // fire a bullet
            if (Mouse::isButtonPressed(sf::Mouse::Left))
            {
                if (gameTimeTotal.asMilliseconds() - lastPressed.asMilliseconds() > 1000 / fireRate 
                    && bulletsInClip > 0)
                {
                    // pass center of player and center of crosshair to the shoot function
                    bullets[currentBullet].shoot(
                        player.getCenter().x, player.getCenter().y, mouseWorldPosition.x, mouseWorldPosition.y
                    );
                    currentBullet += 1;
                    if (currentBullet > 99)
                    {
                        currentBullet = 0;
                    }
                    lastPressed = gameTimeTotal;
                    bulletsInClip--;
                }
            } // end fire a bullet

        } // end WASD while playing

        // Handle Leveling up
        if (state == State::LEVELING_UP)
        {
            if (event.key.code == Keyboard::Num1)
            {
                state = State::PLAYING;
            }

            if (event.key.code == Keyboard::Num2)
            {
                state = State::PLAYING;
            }

            if (event.key.code == Keyboard::Num3)
            {
                state = State::PLAYING;
            }

            if (event.key.code == Keyboard::Num4)
            {
                state = State::PLAYING;
            }

            if (event.key.code == Keyboard::Num5)
            {
                state = State::PLAYING;
            }

            if (event.key.code == Keyboard::Num6)
            {
                state = State::PLAYING;
            }

            if (state == State::PLAYING)
            {
                // Prepare level
                arena.width = 500;
                arena.height = 500;
                arena.left = 0;
                arena.top = 0;
                int tileSize = createBackground(background, arena);

                // spawn player in middle of arena
                player.spawn(arena, resolution, tileSize);

                // create zombie horde
                numZombies = 10;
                delete[] zombies; // delete previously allocated memory
                zombies = createHorde(numZombies, arena);
                numZombiesAlive = numZombies;

                // reset clock so there is no frame jump
                clock.restart();
            }
        } // end levelling up

        // update frame
        if (state == State::PLAYING)
        {
            Time dt = clock.restart();
            gameTimeTotal += dt;
            float dtAsSeconds = dt.asSeconds();
            // where is the mouse pointer
            mouseScreenPosition = Mouse::getPosition();
            mouseWorldPosition = window.mapPixelToCoords(Mouse::getPosition(), mainView);
            spriteCrosshair.setPosition(mouseWorldPosition);
            player.update(dtAsSeconds, Mouse::getPosition());
            Vector2f playerPosition(player.getCenter());
            mainView.setCenter(player.getCenter());
            for (int i = 0; i < numZombies; i++)
            {
                if (zombies[i].isAlive())
                {
                    zombies[i].update(dt.asSeconds(), playerPosition);
                }
            }
            for (int i = 0; i < 100; i++)
            {
                if (bullets[i].isInFlight())
                {
                    bullets[i].update(dtAsSeconds);
                }
            }
            healthPickup.update(dtAsSeconds);
            ammoPickup.update(dtAsSeconds);

            // collision detection
            // have any zombies been shot?
            for (int i = 0; i < 100; i++)
            {
                for (int j = 0; j < numZombies; j++)
                {
                    if (bullets[i].isInFlight() && zombies[j].isAlive())
                    {
                        if (bullets[i].getPosition().intersects(zombies[j].getPosition()))
                        {
                            // stop bullet
                            bullets[i].stop();
                            // register hit and see if it was a kill
                            if (zombies[j].hit())
                            {
                                score += 10;
                                if (score > highScore)
                                {
                                    highScore = score;
                                }
                                numZombiesAlive--;
                                if (numZombiesAlive == 0)
                                {
                                    state = State::LEVELING_UP;
                                }
                            }
                        }
                    }
                }
            } // end zombie being shot

            // have any zombies touched the player?
            for (int i = 0; i < numZombies; i++)
            {
                if (player.getPosition().intersects(zombies[i].getPosition()) && zombies[i].isAlive())
                {
                    if (player.hit(gameTimeTotal))
                    {
                        // more later
                    }

                    if (player.getHealth() <= 0)
                    {
                        state = State::GAME_OVER;
                    }
                }
            }// end player touched

            // has the player touched a health pickup
            if (player.getPosition().intersects(healthPickup.getPosition()) && healthPickup.isSpawned())
            {
                player.increaseHealthLevel(healthPickup.gotIt());
            }

            // has the player touched an ammo pickup
            if (player.getPosition().intersects(ammoPickup.getPosition()) && ammoPickup.isSpawned())
            {
                bulletsSpare += ammoPickup.gotIt();
            }

            // health bar
            healthBar.setSize(Vector2f(player.getHealth() * 3, 50));

            // increment frames since the previous update
            framesSinceLastHUDUpdate++;

            // recalculate every fpsMeasurementFrameInterval frames
            if (framesSinceLastHUDUpdate > fpsMeasurementFrameInterval)
            {
                // Update game HUD text
                std::stringstream ssAmmo;
                std::stringstream ssScore;
                std::stringstream ssScore;
                std::stringstream ssHighScore;
                std::stringstream ssWave;
                std::stringstream ssZombiesAlive;

                // update ammo text
                ssAmmo << bulletsInClip << "/" << bulletsSpare;
                ammoText.setString(ssAmmo.str());

                // update score text
                ssScore << "Score: " << score;
                scoreText.setString(ssScore.str());

                // update high score text
                ssHighScore << "High Score: " << highScore;
                highScoreText.setString(ssHighScore.str());

                // update wave text
                ssWave << "Wave: " << wave;
                waveNumberText.setString(ssWave.str());

                // update zombies alive text
                ssZombiesAlive << "Zombies Alive: " << numZombiesAlive;
                zombiesRemainingText.setString(ssZombiesAlive.str());

                framesSinceLastHUDUpdate = 0;
            } // end HUD update

        } // end updating frame

        // draw scene
        if (state == State::PLAYING)
        {
            window.clear();
            
            window.setView(mainView);
            window.draw(background, &textureBackground);
            for (int i = 0; i < numZombies; i++)
            {
                window.draw(zombies[i].getSprite());
            }
            for (int i = 0; i < 100; i++)
            {
                if (bullets[i].isInFlight())
                {
                    window.draw(bullets[i].getShape());
                }
            }
            window.draw(player.getSprite());
            window.draw(spriteCrosshair);
            if (ammoPickup.isSpawned())
            {
                window.draw(ammoPickup.getSprite());
            }
            if (healthPickup.isSpawned())
            {
                window.draw(healthPickup.getSprite());
            }

            // switch to HUD view and draw HUD elements
            window.setView(hudView);
            window.draw(spriteAmmoIcon);
            window.draw(ammoText);
            window.draw(scoreText);
            window.draw(highScoreText);
            window.draw(healthBar);
            window.draw(waveNumberText);
            window.draw(zombiesRemainingText);
        }

        if (state == State::LEVELING_UP)
        {
            window.draw(spriteGameOver);
            window.draw(levelUpText);
        }

        if (state == State::PAUSED)
        {
            window.draw(pausedText);
        }

        if (state == State::GAME_OVER)
        {
            window.draw(spriteGameOver);
            window.draw(gameOverText);
            window.draw(scoreText);
            window.draw(highScoreText);
        }

        window.display();

    }// End game loop

    delete[] zombies; // delete previously allocated memory

    return 0;
}