#include "characters/Player.hpp"
#include "gameDimensions.hpp"
#include "physics/physicsConstants.hpp"
#include <iostream>
#include <cmath>
#include <vector>

#include <cmath>

const int GROUND_HEIGHT = 608; //This is just the current ground height based on how player position is called in GameLogic
const float JUMP_HEIGHT = 100.0f;

int Player::getCurrentAnimationOffset() const {
    return (animationTicks % 40) / 10;
}

Uint32 onInvincibilityEnd(Uint32 interval, void *param) {
    auto* player = reinterpret_cast<Player*>(param);

    player->setInvincible(false);

    return 0;
}

void Player::move(double ms) {
    // Basic character movement
    double seconds = ms / 1000;

    // Only apply vertical acceleration if the player is not on the ground
    if (!onGround) {
        velocity.setY(velocity.getY() + GRAVITY * seconds);

        // Once the jump has reached its maximum height, start falling
        if (isJumping && velocity.getY() > 0) {
            falling = true;
        }
    }

    position += velocity * seconds;

    // Deal with more advanced functionality
    if (currentDirection != MoveDirection::NONE) {
        animationTicks++;
    }

    handleCollisions();
    checkForFallRespawn();

    // Move the projectiles, while marking any which should be deleted
    std::vector<size_t> toDelete;

    for (size_t idx = 0; idx < projectiles.size(); idx++) {
        auto& proj = projectiles[idx];

        if (proj.isActive()) {
            proj.move(ms);
        } else {
            toDelete.push_back(idx);
        }
    }

    // Delete the corresponding indexes
    if (toDelete.size() > 0) {
        // This is needed to keep the indexes accurate
        int deleted = 0;

        for (auto idx : toDelete) {
            projectiles.erase(projectiles.begin() + idx - deleted);
            deleted++;
        }
    }
}

void Player::checkForFallRespawn() {
    if (onGround) {
        respawnPos = position;
    }

    if (position.getY() > offMapHeight) {
        SoundManager::getInstance()->playSound(SoundEffect::JUMP); //CHANGE TO SOME FALL SOUND AND MAYBE ADJUST SO IT PLAYS WHILE PLAYER IS FALLING
        respawn();
    }
}

void Player::respawn() {

    if (fallDirection == MoveDirection::RIGHT) {
        respawnPos.setX(respawnPos.getX()-10);
        // respawnPos.setY(respawnPos.getY()-10);
    }
    else if (fallDirection ==MoveDirection::LEFT) {
        respawnPos.setX(respawnPos.getX()+10);

    }

    gameLogic.getTimer()->subtractTime(10);
    invincibilityFramesActive = true;
    invincibilityTimerId = SDL_AddTimer(INVINCIBILITY_FRAMES, onInvincibilityEnd, this);

    position = respawnPos;
    velocity = Vector2(0, 0);
    onGround=true;


    std::cout << "Player respawned at: " << position << std::endl;
}

BoundingBox Player::getHitbox() const {
    // The hitbox is different if the player is flipped
    if (lastDirection == MoveDirection::RIGHT) {
        return hitbox;
    } else {
        auto oldOffset = hitbox.getOffset();

        return BoundingBox(
            Vector2(-12, oldOffset.getY()), // -12 is a magic number here, but it is the correct offset for flipping the player sprite
            hitbox.getSize()
        );
    }
}

// Callback for the projectile timer
Uint32 onProjectileTimer(Uint32 interval, void *param) {
    auto* player = reinterpret_cast<Player*>(param);

    player->setIfProjectileTimerActive(false);

    return 0; // Don't repeat
}

void Player::shoot() {
    if (isProjectileTimerActive) {
        return;
    }

    SoundManager::getInstance()->playSound(SoundEffect::SHOOT);

    auto newProjectile = Projectile(std::make_shared<GameLogic>(gameLogic), position, currentDirection);

    if (currentDirection == MoveDirection::LEFT) {
        newProjectile.setStartingPosition(currentDirection);
        newProjectile.setVelocity(-300, 0);
    } else if (currentDirection == MoveDirection::RIGHT) {
        newProjectile.setStartingPosition(currentDirection);
        newProjectile.setVelocity(300, 0);
    } else if (lastDirection == MoveDirection::LEFT) {
        newProjectile.setStartingPosition(lastDirection);
        newProjectile.setVelocity(-300, 0);
    } else {
        newProjectile.setStartingPosition(MoveDirection::RIGHT);
        newProjectile.setVelocity(300, 0);
    }

    // Add to the list of projectiles if there aren't already too many
    if (projectiles.size() < MAX_PROJECTILES) {
        projectiles.push_back(newProjectile);
    }

    // Set up the projectile timer
    isProjectileTimerActive = true;
    projectileTimerId = SDL_AddTimer(PROJECTILE_DELAY, onProjectileTimer, this);
}

void Player::stopMoving() {
    velocity.setX(0);

    currentDirection = MoveDirection::NONE;
    animationTicks = 0;
    // velocity = Vector2(0, 0);
}

void Player::moveLeft() {
    float speed = -getCurrentSpeed();
    velocity.setX(speed);
    currentDirection = MoveDirection::LEFT;
    lastDirection = MoveDirection::LEFT;
}

void Player::moveRight() {
    float speed = getCurrentSpeed();
    velocity.setX(speed);
    currentDirection = MoveDirection::RIGHT;
    lastDirection = MoveDirection::RIGHT;
}

void Player::jump() {
    if (velocity.getY() == 0) {
        SoundManager::getInstance()->playSound(SoundEffect::JUMP);
        std::cout << "jump" << std::endl;
        velocity.setY(-500);
        position -= Vector2(0, 1); // Update position to avoid an immediate collision with the ground
        bufferedJump = false;
        onGround= false;
        isJumping = true;
        falling = false;
        fallDirection = currentDirection;

        // Fall height isn't applicable
        fallHeight = -1000;
    } else {
        bufferedJump = true;
    }
}

Vector2 Player::getHitboxCenter() const {
    return position + hitbox.getOffset() + hitbox.getSize() / 2.0;
}

Uint32 onSpeedSlowEnd(Uint32 interval, void *param) {
    auto* player = reinterpret_cast<Player*>(param);
    player->restoreSpeed();
    return 0;
}
Uint32 onSpeedFastEnd(Uint32 interval, void *param) {
    auto* player = reinterpret_cast<Player*>(param);
    player->restoreSpeed();
    return 0;
}

void Player::reduceSpeed() {
    if (!isSlowed) {
        std::cout<<"reducing speed"<<std::endl;


        if (velocity.getX() > 0) {
            velocity.setX(REDUCED_SPEED);
        } else if (velocity.getX() < 0) {
            velocity.setX(-REDUCED_SPEED);
        }

        isSlowed = true;

        slowTimerId = SDL_AddTimer(SPEED_FRAMES, onSpeedSlowEnd, this);

    } else {
        SDL_RemoveTimer(slowTimerId);
        slowTimerId = SDL_AddTimer(SPEED_FRAMES, onSpeedSlowEnd, this);
    }
}
void Player:: increaseSpeed() {
    if (!isFast) {
        std::cout<<"increasing speed"<<std::endl;

        if (velocity.getX() > 0) {
            velocity.setX(INCREASED_SPEED);
        } else if (velocity.getX() < 0) {
            velocity.setX(-INCREASED_SPEED);
        }

        isFast = true;

        fastTimerId = SDL_AddTimer(SPEED_FRAMES, onSpeedFastEnd, this);
    } else {
        // If the timer is already on, just extend the timer
        SDL_RemoveTimer(fastTimerId);
        fastTimerId = SDL_AddTimer(SPEED_FRAMES, onSpeedFastEnd, this);
    }
}
void Player::restoreSpeed() {
    if (onGround) {
        isSlowed = false;
        isFast = false;
        std::cout << "Reset speed to normal" << std::endl;
    } else {
        restoreSpeedWhenLand = true;
    }
}

float Player::getCurrentSpeed() const {
    if (isFast) {
        return INCREASED_SPEED;
    } else if (isSlowed) {
        return REDUCED_SPEED;
    } else {
        return NORMAL_SPEED;
    }
}

void Player::handleFloorCollisions() {
    // If the player is on the ground, confirm that they are still on the ground by checking for a collision
    // Otherwise, check if the player has collided with the ground. If they have, set them as on the ground and update their position + velocity. If they have a buffered jump, then jump.

    /**
     * if onGround {
     *   if player is not on the ground {
     *     set onGround to false
     *     set falling to true
     *   }
     * } else {
     *   if player is colliding with the ground {
     *     if falling and the collided tile is on the same y-level as the original falling location {
     *       continue
     *     } else if jumping and not falling {
     *       continue
     *     } else if bufferedJump {
     *       jump (should set isJumping to true)
     *     } else {
     *       set position to be on top of the block
     *       reset vertical velocity
     *       set onGround to true
     *     }
     *   }
     * }
     *
     */

    auto hitbox = getHitbox() + position;

    auto level = gameLogic.getLevel();
    auto topY = hitbox.getTopY();
    auto bottomY = hitbox.getBottomY();
    auto leftX = hitbox.getLeftX();
    auto rightX = hitbox.getRightX();

    // std::cout << position << ", " << hitbox.getOffset() << std::endl;

     if (onGround) {
        bool isOnGround = false;

        for (auto x = leftX; x <= rightX; x += TILE_SIZE / 2) {
            auto worldTile = level->getWorldCollisionObject(Vector2(floor(x / TILE_SIZE), floor(bottomY / TILE_SIZE)));

            if (worldTile) {
                if(worldTile->type == "Obstacle") {
                    reduceSpeed();
                }

                if (fabs(worldTile->bounds.y - bottomY) <= 4)
                    isOnGround = true;

                break;
            }
        }

        if (!isOnGround) {
            // Start falling
            onGround = false;
            falling = true;
            fallDirection = currentDirection;
            fallHeight = bottomY;
        }
     } else {
        for (auto x = leftX; x <= rightX; x += TILE_SIZE / 2) {
            auto tileX = floor(x / TILE_SIZE);
            auto tileY = floor(bottomY / TILE_SIZE);
            auto worldTile = level->getWorldCollisionObject(Vector2(tileX, tileY));

            // This line is the key
            if (worldTile) {
                auto bounds = worldTile->bounds;

                // Check if the player is too far left/right
                if (bounds.x >= rightX || (bounds.x + bounds.w <= leftX)) {
                    continue;
                }

                // If partOfWall is true, then we are against a wall and should not be able to jump or land.
                auto partOfWall = level->getWorldCollisionObject(Vector2(tileX, tileY - 1));

                auto tilePos = worldTile->bounds;

                // Falling off the same height
                if (falling && tilePos.y == fallHeight) {
                    continue;
                } else if (isJumping && !falling) {
                    continue;
                } else if (bufferedJump) {
                    jump();
                }
                // std::cout << tilePos.y << ", " << bottomY << std::endl;

                if (!partOfWall) {
                    // std::cout << "Hit ground" << std::endl;
                    position.setY(tilePos.y - PLAYER_HEIGHT / 2);
                    velocity.setY(0);
                    isJumping = false;
                    onGround = true;
                    falling = false;

                    if (restoreSpeedWhenLand) {
                        isSlowed = false;
                        isFast = false;
                        restoreSpeedWhenLand = false;
                    }
                    break;
                }
            }
        }
     }
}

void Player::handleCeilingCollisions() {
    auto hitbox = getHitbox() + position;
    auto level = gameLogic.getLevel();

    auto hitboxWidth = hitbox.getSize().getX();
    auto hitboxHeight = hitbox.getSize().getY();

    auto topY = hitbox.getTopY();
    auto bottomY = hitbox.getBottomY();
    auto leftX = hitbox.getLeftX();
    auto rightX = hitbox.getRightX();
    int unused;

    // The idea is to factor in if the player is moving in a direction
    for (auto x = (currentDirection == MoveDirection::LEFT ? leftX + 4 : leftX); x <= (currentDirection == MoveDirection::RIGHT ? rightX - 8 : rightX); x += TILE_SIZE / 2) {
        auto collideWorld = level->getWorldCollisionObject(Vector2(floor(x / TILE_SIZE), floor(topY / TILE_SIZE)));

        if (collideWorld) {
            auto bounds = collideWorld->bounds;

            // Check if the player is too far left/right
            if (bounds.x >= rightX || (bounds.x + bounds.w <= leftX)) {
                continue;
            }

            // std::cout << "ceiling collision" << std::endl;
            // position.setX(collideWorld->bounds.x - hitboxWidth);
            position.setY(collideWorld->bounds.y + collideWorld->bounds.h + PLAYER_HEIGHT / 2 + 1);
            velocity.setY(0.1);
            falling = true;
            break;

        }
    }
}

void Player::handleRightCollisions() {
    auto hitbox = getHitbox() + position;
    auto level = gameLogic.getLevel();

    auto hitboxWidth = hitbox.getSize().getX();
    auto hitboxHeight = hitbox.getSize().getY();

    auto topY = hitbox.getTopY();
    auto bottomY = hitbox.getBottomY();
    auto leftX = hitbox.getLeftX();
    auto rightX = hitbox.getRightX();

    for (auto y = topY; y <= bottomY - 1; y += TILE_SIZE / 2) {
        auto collideWorld = level -> getWorldCollisionObject(Vector2(floor(rightX / TILE_SIZE), floor(y / TILE_SIZE)));

        if (collideWorld) {
            auto bounds = collideWorld->bounds;

            // The idea is to not have a collision because the block is too low/high
            if (bounds.y >= bottomY || (bounds.y + bounds.h <= topY)) {
                continue;
            }

            // std::cout<<"Right Collision Detected"<<std::endl;
            // std::cout<<"X position"<< position.getX()<<" tile X; "<< collideWorld->bounds.x<<"player width" <<hitboxWidth<<" rightX *32 "<<rightX*TILE_SIZE<<std::endl;/
            // position.setX(collideWorld->bounds.x - hitboxWidth);
            position.setX(collideWorld->bounds.x - PLAYER_WIDTH / 2 - 1);
            velocity.setX(0);
            break;

        }
    }
}

void Player::handleLeftCollisions() {
    auto hitbox = getHitbox() + position;
    auto level = gameLogic.getLevel();

    auto hitboxWidth = hitbox.getSize().getX();
    auto hitboxHeight = hitbox.getSize().getY();

    auto topY = hitbox.getTopY();
    auto bottomY = hitbox.getBottomY();
    auto leftX = hitbox.getLeftX();
    auto rightX = hitbox.getRightX();

    // Check collisions with the side wall
    if (leftX < 0) {
        position.setX(PLAYER_WIDTH / 2 + 1);
    }

    for (auto y = topY; y <= bottomY-1; y += TILE_SIZE / 2) {
        auto collideWorld = level -> getWorldCollisionObject(Vector2(floor(leftX / TILE_SIZE), floor(y / TILE_SIZE)));

        if (collideWorld) {
            auto bounds = collideWorld->bounds;

            // The idea is to not have a collision because the block is too low/high
            if (bounds.y >= bottomY || (bounds.y + bounds.h <= topY)) {
                continue;
            }

            // std::cout<<"Left Collision Detected"<<std::endl;
            // std::cout<<"X position"<< position.getX()<<" tile X; "<< collideWorld->bounds.x+collideWorld->bounds.h<<"player width" <<hitboxWidth<<" leftX*32 "<<leftX*TILE_SIZE<<std::endl;
            position.setX(collideWorld->bounds.x + collideWorld->bounds.w + PLAYER_WIDTH / 2 + 1);
            // position.setX(collideWorld->bounds.x+collideWorld->bounds.w+(PLAYER_WIDTH/2)+1);
            velocity.setX(0);
        }
    }
}




void Player::handleEnemyCollisions() {
    auto enemies = gameLogic.getLevel()->getEnemies();

    for (auto enemy : enemies){
        auto playerHitbox = getHitbox() + position;
        auto enemyHitbox = enemy->getHitbox() + enemy->getPosition();

        // Detect if the 2 bounding boxes overlap
        if (playerHitbox.overlaps(enemyHitbox)) {
            // std::cout << "enemy collision" << std::endl;
            gameLogic.getTimer()->subtractTime(5); // right now all enemy collisions are 5 seconds
            invincibilityFramesActive = true;
            invincibilityTimerId = SDL_AddTimer(INVINCIBILITY_FRAMES, onInvincibilityEnd, this);
            break;
        }
    }

    // Projectile collisions
    for (auto enemy : enemies) {
        for (auto& projectile : enemy->getProjectiles()) {
            auto playerHitbox = getHitbox() + position;
            auto projHitbox = projectile.getHitbox() + projectile.getPosition();

            if (playerHitbox.overlaps(projHitbox)) {
                std::cout<<enemy->getPosition().getX()<<"WHY"<<std::endl;
                reduceSpeed();
                SoundManager::getInstance()->playSound(SoundEffect::DAMAGE); 
                projectile.setActive(false);
            }
        }
    }
}

void Player::handleCollisions() {
    /**
     * if player is hitting an obstacle to the right and is moving right, push them back
     * if player is hitting an obstacle to the left and is moving left, push them back
     * check bottom tiles
     * check top tiles
     *
     * To fix one issue, if the player is falling off of a structure, don't check vertically unless the new position is lower
     * If the player is jumping, only check vertically once they reach the peak of their jump
     */


     handleFloorCollisions();

     if (velocity.getY() < 0)
        handleCeilingCollisions();

     if (velocity.getX() > 0) {
        handleRightCollisions();
     } else if (velocity.getX() < 0) {
        handleLeftCollisions();
     }

    if (!invincibilityFramesActive) {
        handleEnemyCollisions();
    }
    handlePowerupCollisions();

    // Check for reaching the end of the level
    auto level = gameLogic.getLevel();

    if (position.getX() >= level->getLevelEndPos()) {
        // Reached the end of the level
        gameLogic.stopLevelReachedEnd();
    }
}
void Player::handlePowerupCollisions() {
    auto powerups = gameLogic.getLevel()->getPowerups();

    for (auto powerup : powerups){
        auto playerHitbox = getHitbox() + position;
        auto powerupHitbox = powerup->getHitbox() + powerup->getPosition();

        // Detect if the 2 bounding boxes overlap
        if (playerHitbox.overlaps(powerupHitbox)) {
            std::cout << "powewrup collision" << std::endl;
            increaseSpeed();
            SoundManager::getInstance()->playSound(SoundEffect::POWERUP); 
            powerup->deactivate();


        }
    }
}
