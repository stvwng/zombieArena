#include "Zombie.h"
#include "TextureHolder.h"
#include <cstdlib>
#include <ctime>
#include <cmath>

using namespace std;

void Zombie::spawn(float startX, float startY, int type, int seed)
{
    switch (type)
    {
        case 0:
            // Bloater
            m_Sprite = Sprite(TextureHolder::GetTexture("graphics/bloater.png"));
            m_Speed = BLOATER_SPEED;
            m_Health = BLOATER_HEALTH;
            break;

        case 1:
            // Chaser
            m_Sprite = Sprite(TextureHolder::GetTexture("graphics/chaser.png"));
            m_Speed = CHASER_SPEED;
            m_Health = CHASER_HEALTH;
            break;

        case 2:
            // Crawler
            m_Sprite = Sprite(TextureHolder::GetTexture("graphics/crawler.png"));
            m_Speed = CRAWLER_SPEED;
            m_Health = CRAWLER_HEALTH;
            break;
    }

    // speed modifier
    srand((int)time(0) * seed);
    float modifier = (rand() % MAX_VARIANCE) + OFFSET;
    modifier /= 100;
    m_Speed *= modifier;

    // initialize location and set position
    m_Position.x = startX;
    m_Position.y = startY;
    m_Sprite.setOrigin(25, 25);
    m_Sprite.setPosition(m_Position);
}

bool Zombie::hit()
{
    m_Health--;
    
    if (m_Health < 0)
    {
        m_Alive = false;
        m_Sprite.setTexture(TextureHolder::GetTexture("graphics/blood.png"));
        return true;
    }

    return false;
}

bool Zombie::isAlive()
{
    return m_Alive;
}

FloatRect Zombie::getPosition()
{
    return m_Sprite.getGlobalBounds();
}

Sprite Zombie::getSprite()
{
    return m_Sprite;
}

void Zombie::update(float elapsedTime, Vector2f playerLocation)
{
    float playerX = playerLocation.x;
    float playerY = playerLocation.y;

    if (playerX > m_Position.x)
    {
        m_Position.x = m_Position.x + m_Speed * elapsedTime;
    }
    else if (playerX < m_Position.x)
    {
        m_Position.x = m_Position.x - m_Speed * elapsedTime;
    }

    if (playerY > m_Position.y)
    {
        m_Position.y = m_Position.y + m_Speed * elapsedTime;
    }
    else if (playerY < m_Position.y)
    {
        m_Position.y = m_Position.y - m_Speed * elapsedTime;
    }

    m_Sprite.setPosition(m_Position);
    // face Sprite in right direction
    float angle = (atan2(playerY - m_Position.y, playerX - m_Position.x) * 180) / 3.141;
    m_Sprite.setRotation(angle);
}