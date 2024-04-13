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
        }

        if (state == State::LEVELING_UP)
        {

        }

        if (state == State::PAUSED)
        {

        }

        if (state == State::GAME_OVER)
        {

        }

        window.display();

    }// End game loop

    delete[] zombies; // delete previously allocated memory

    return 0;
}