#include "EnemyProjectile.hpp"
#include "gameDimensions.hpp"
#include <cmath>
#include <iostream>
#include "GameLogic.hpp"


EnemyProjectile::EnemyProjectile(std::shared_ptr<GameLogic> _gameLogic, Vector2 playerPosition, Vector2 enemyPosition) : 
    gameLogic(_gameLogic),
    currentPosition(enemyPosition), 
    direction((playerPosition - enemyPosition).normal() * 300),
    active(true) {}


void EnemyProjectile::move(double ms) {
    double seconds = ms/1000;

    double moveOffsetX = direction.getX() * seconds;
    double moveOffsetY = direction.getY() * seconds;


    //double moveOffset = velocity.getX() * seconds;
    currentPosition.setX(currentPosition.getX() + moveOffsetX);
    currentPosition.setY(currentPosition.getY() + moveOffsetY);

    enemyProjTraveledDist += 300 * seconds;

    if (enemyProjTraveledDist >= 500) {
        active = false;
    }

    // Detect collisions with walls
    auto hitboxPos = hitbox + currentPosition;
    auto leftX = hitboxPos.getLeftX();
    auto rightX = hitboxPos.getRightX();
    auto topY = hitboxPos.getTopY();
    auto bottomY = hitboxPos.getBottomY();
    
    auto level = gameLogic->getLevel();

    for (auto x : {leftX, rightX}) {
        for (auto y : {topY, bottomY}) {
            auto worldTile = level->getWorldCollisionObject(Vector2(
                floor(x / TILE_SIZE),
                floor(y / TILE_SIZE)
            ));

            if (worldTile) {
                if (hitboxPos.overlaps(BoundingBox(
                    Vector2(worldTile->bounds.x, worldTile->bounds.y),
                    Vector2(worldTile->bounds.w, worldTile->bounds.h)
                ))) {
                    active = false;
                    return;
                }
            }
        }
    }
}
