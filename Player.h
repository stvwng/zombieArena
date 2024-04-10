#pragma once
#include <SFML/Graphics.hpp>

using namespace sf;

class Player
{
    private:
        const float START_SPEED = 200;
        const float START_HEALTH = 100;

        Vector2f m_Position;
        Sprite m_Sprite;
        Texture m_Texture;
        Vector2f m_Resolution; // screen resolution
        IntRect m_Arena;
        int m_TileSize;
        bool m_UpPressed;
        bool m_DownPressed;
        bool m_LeftPressed;
        bool m_RightPressed;
        int m_Health;
        int m_MaxHealth;
        Time m_LastHit; // time Player was last hit
        float m_Speed; // speed in pixels per second

    public:
        Player();
        void spawn(IntRect arena, Vector2f resolution, int tileSize);
        void resetPlayerStats();
        bool hit(Time timeHit); // handle player getting hit by a zombie
        Time getLastTimeHit();
        FloatRect getPosition();
        Vector2f getCenter();
        float getRotation(); // what angle is the Player facing?
        Sprite getSprite();
        void moveUp();
        void moveDown();
        void moveLeft();
        void moveRight();
        void stopUp();
        void stopDown();
        void stopLeft();
        void stopRight();
        void update(float elapsedTime, Vector2i mousePosition);
        void upgradeSpeed();
        void upgradeHealth();
        void increaseHealthLevel(int amount);
        int getHealth();
};