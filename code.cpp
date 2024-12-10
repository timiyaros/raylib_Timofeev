#include "raylib.h"
#include <math.h>
#include <stdlib.h>

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080
#define PLAYER_SPEED 5.0f
#define BULLET_SPEED 10.0f
#define BASE_TARGET_SPEED 3.0f  // Начальная скорость врагов
#define MAX_ENEMIES 100         // Максимальное количество врагов
#define ENEMY_SIZE 30
#define MAX_BULLETS 10         // Максимальное количество пуль
#define ATTACK_RADIUS 100      // Радиус атаки
#define ATTACK_DURATION 0.5f   // Продолжительность атаки (в секундах)

typedef struct {
    Vector2 position;
    Vector2 velocity;
    bool active;
    int type;  // Тип врага
} Enemy;

typedef struct {
    Vector2 position;
    Vector2 velocity;
    bool active;
} Bullet;

Enemy enemies[MAX_ENEMIES];
Bullet bullets[MAX_BULLETS];

Vector2 playerPos = { 910.0f, 510.0f };  // Позиция игрока
int health = 3;             // ХП игрока
int shirHealth = 150;       // Ширина ХП
int count = 0;              // Счётчик убийств
bool game = true;           // Игра продолжается

float enemySpeed = BASE_TARGET_SPEED; // Начальная скорость врагов
int enemyCount = 15;                // Начальное количество врагов
float enemySpawnRate = 0.7f;        // Время между спауном
float timeSinceLastSpawn = 0.0f;    // Время с последнего спауна врагов

bool paused = false;  // Флаг для паузы

// Для атаки
bool attacking = false;         // Флаг атаки
float attackTimer = 0.0f;       // Таймер атаки

// Нормализация вектора
Vector2 NormalizeVector(Vector2 vec) {
    float length = sqrtf(vec.x * vec.x + vec.y * vec.y);
    if (length > 0) {
        vec.x /= length;
        vec.y /= length;
    }
    return vec;
}

// Функция для проверки, уничтожены ли все враги
bool AreAllEnemiesDead() {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active) {
            return false;  // Если хоть один враг активен, возвращаем false
        }
    }
    return true;  // Все враги мертвы
}

// Функция для инициализации врага
void SpawnEnemy() {
    // По мере увеличения количества врагов, их спавн будет происходить чаще
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) {
            enemies[i].active = true;

            // Рандомный спавн за пределами экрана
            int side = GetRandomValue(1, 4);
            if (side == 1) {  // Слева
                enemies[i].position.x = -10;
                enemies[i].position.y = GetRandomValue(0, SCREEN_HEIGHT);
            }
            if (side == 2) {  // Сверху
                enemies[i].position.x = GetRandomValue(0, SCREEN_WIDTH);
                enemies[i].position.y = -10;
            }
            if (side == 3) {  // Справа
                enemies[i].position.x = SCREEN_WIDTH + 10;
                enemies[i].position.y = GetRandomValue(0, SCREEN_HEIGHT);
            }
            if (side == 4) {  // Снизу
                enemies[i].position.x = GetRandomValue(0, SCREEN_WIDTH);
                enemies[i].position.y = SCREEN_HEIGHT + 10;
            }

            // Разные типы врагов (например, медленные и быстрые)
            enemies[i].type = GetRandomValue(0, 1);  // Тип 0 - обычный, 1 - быстрый

            enemies[i].velocity = Vector2{ 0 };
            break;
        }
    }
}

// Функция для проверки столкновения двух врагов
bool CheckCollisionBetweenEnemies(Enemy enemy1, Enemy enemy2) {
    return CheckCollisionRecs(Rectangle{
        enemy1.position.x, enemy1.position.y, ENEMY_SIZE, ENEMY_SIZE
        }, Rectangle{
            enemy2.position.x, enemy2.position.y, ENEMY_SIZE, ENEMY_SIZE
        });
}

// Функция для корректировки позиции врага, если происходит столкновение
void HandleEnemyCollision(Enemy* enemy1, Enemy* enemy2) {
    // Простейшая коррекция - сдвигаем врага вбок
    if (CheckCollisionBetweenEnemies(*enemy1, *enemy2)) {
        if (enemy1->position.x < enemy2->position.x) {
            enemy1->position.x -= 5;  // Сдвигаем влево
        }
        else {
            enemy1->position.x += 5;  // Сдвигаем вправо
        }

        if (enemy1->position.y < enemy2->position.y) {
            enemy1->position.y -= 5;  // Сдвигаем вверх
        }
        else {
            enemy1->position.y += 5;  // Сдвигаем вниз
        }
    }
}

// Получение ближайшего врага
Vector2 GetNearestEnemy() {
    float minDistance = 1e6;
    Vector2 nearestPos = { 0, 0 };

    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active) {
            float dist = sqrtf((enemies[i].position.x - playerPos.x) * (enemies[i].position.x - playerPos.x) +
                (enemies[i].position.y - playerPos.y) * (enemies[i].position.y - playerPos.y));
            if (dist < minDistance) {
                minDistance = dist;
                nearestPos = enemies[i].position;
            }
        }
    }
    return nearestPos;
}

// Функция для атаки
void PerformAttack() {
    attacking = true;
    attackTimer = ATTACK_DURATION;  // Устанавливаем время атаки
}

// Функция для обработки атаки
void UpdateAttack(float deltaTime) {
    if (attacking) {
        attackTimer -= deltaTime;

        if (attackTimer <= 0) {
            attacking = false;  // Завершаем атаку, когда таймер истекает
        }
    }
}

// Функция для проверки попадания врага в круг атаки
bool CheckEnemyHitAttack(Enemy enemy) {
    float distance = sqrtf((enemy.position.x - playerPos.x) * (enemy.position.x - playerPos.x) +
        (enemy.position.y - playerPos.y) * (enemy.position.y - playerPos.y));

    return distance <= ATTACK_RADIUS;  // Если враг находится в радиусе атаки
}

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Enemy Waves Game");
    SetTargetFPS(60);

    SpawnEnemy(); // Начальный спавн

    while (!WindowShouldClose() && game) {
        // Проверка на паузу
        if (IsKeyPressed(KEY_P)) {
            paused = !paused;  // Переключение паузы
        }

        // Если игра на паузе, ничего не обновляем
        if (!paused) {
            // Обновление времени и проверка необходимости спауна нового врага
            timeSinceLastSpawn += GetFrameTime();

            // Если прошло достаточно времени, то спавним врага
            if (timeSinceLastSpawn >= enemySpawnRate) {
                timeSinceLastSpawn = 0.0f;  // Сбрасываем таймер
                SpawnEnemy();  // Спавним врага

                // Увеличиваем количество врагов на 1 раз за цикл
                if (enemyCount < MAX_ENEMIES) {
                    enemyCount++;  // Увеличиваем количество врагов
                }

                // Немного увеличиваем скорость врагов
                if (enemySpeed < BASE_TARGET_SPEED + 0.4f) { // Увеличение когда
                    enemySpeed += 0.04f;  // Увеличиваем скорость врагов
                }

                // Уменьшаем интервал появления врагов (делаем их более частыми)
                if (enemySpawnRate > 0.3f) {
                    enemySpawnRate -= 0.01f;  // Уменьшаем интервал появления медленно
                }
            }

            // Движение игрока
            if (IsKeyDown(KEY_W) && playerPos.y > 0) playerPos.y -= PLAYER_SPEED;
            if (IsKeyDown(KEY_S) && playerPos.y < SCREEN_HEIGHT - 50) playerPos.y += PLAYER_SPEED;
            if (IsKeyDown(KEY_A) && playerPos.x > 0) playerPos.x -= PLAYER_SPEED;
            if (IsKeyDown(KEY_D) && playerPos.x < SCREEN_WIDTH - 50) playerPos.x += PLAYER_SPEED;

            // Стрельба
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                // Найдем первое пустое место для пули
                for (int i = 0; i < MAX_BULLETS; i++) {
                    if (!bullets[i].active) {
                        bullets[i].position = playerPos;
                        bullets[i].active = true;

                        Vector2 nearestEnemy = GetNearestEnemy();
                        bullets[i].velocity = NormalizeVector(Vector2{ nearestEnemy.x - bullets[i].position.x, nearestEnemy.y - bullets[i].position.y });
                        break; // Выход из цикла, чтобы выпустить только одну пулю за раз
                    }
                }
            }

            // Атака с ПКМ
            if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
                PerformAttack();
            }

            // Обновление состояния атаки
            UpdateAttack(GetFrameTime());

            // Движение пуль
            for (int i = 0; i < MAX_BULLETS; i++) {
                if (bullets[i].active) {
                    bullets[i].position.x += bullets[i].velocity.x * BULLET_SPEED;
                    bullets[i].position.y += bullets[i].velocity.y * BULLET_SPEED;

                    if (bullets[i].position.x < 0 || bullets[i].position.x > SCREEN_WIDTH ||
                        bullets[i].position.y < 0 || bullets[i].position.y > SCREEN_HEIGHT) {
                        bullets[i].active = false;
                    }

                    // Проверка на попадание в врага
                    for (int j = 0; j < MAX_ENEMIES; j++) {
                        if (enemies[j].active && CheckCollisionRecs(Rectangle{ bullets[i].position.x, bullets[i].position.y, 10, 10 },
                            Rectangle{ enemies[j].position.x, enemies[j].position.y, ENEMY_SIZE, ENEMY_SIZE })) {
                            enemies[j].active = false;  // Уничтожаем врага
                            bullets[i].active = false;  // Останавливаем пулю
                            count++; // Увеличиваем счет
                        }
                    }
                }
            }

            // Движение врагов
            for (int i = 0; i < MAX_ENEMIES; i++) {
                if (enemies[i].active) {
                    Vector2 direction = NormalizeVector(Vector2{ playerPos.x - enemies[i].position.x, playerPos.y - enemies[i].position.y });
                    enemies[i].position.x += direction.x * enemySpeed;
                    enemies[i].position.y += direction.y * enemySpeed;

                    // Проверка на столкновение с игроком
                    if (CheckCollisionRecs(Rectangle{ playerPos.x, playerPos.y, 50, 50 },
                        Rectangle{ enemies[i].position.x, enemies[i].position.y, ENEMY_SIZE, ENEMY_SIZE })) {
                        enemies[i].active = false;
                        health--;
                        shirHealth -= 50;
                        if (health <= 0) game = false;
                    }

                    // Проверка попадания врагов в круг атаки
                    if (attacking && CheckEnemyHitAttack(enemies[i])) {
                        enemies[i].active = false;  // Уничтожаем врага, если он в зоне атаки
                    }
                }
            }
        }

        // Отрисовка
        BeginDrawing();
        ClearBackground(BLACK);

        // Отображаем игрока
        DrawRectangleV(playerPos, Vector2{ 50, 50 }, BLUE);

        // Отображаем пули
        for (int i = 0; i < MAX_BULLETS; i++) {
            if (bullets[i].active) {
                DrawRectangleV(bullets[i].position, Vector2{ 10, 10 }, YELLOW);
            }
        }

        // Отображаем врагов
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (enemies[i].active) {
                DrawRectangleV(enemies[i].position, Vector2{ ENEMY_SIZE, ENEMY_SIZE }, RED);
            }
        }

        // Отображаем ХП и количество убийств
        DrawText(TextFormat("Health: %i", health), 10, 10, 20, WHITE);
        DrawText(TextFormat("Kills: %i", count), 10, 40, 20, WHITE);
        DrawRectangle(10, 80, shirHealth, 20, RED);  // ПАНЕЛЬ ХП

        // Отображаем паузу
        if (paused) {
            DrawText("PAUSED", SCREEN_WIDTH / 2 - 80, SCREEN_HEIGHT / 2, 40, RED);
        }

        // Отображаем круг атаки, если активен
        if (attacking) {
            DrawCircleV({playerPos.x+25,playerPos.y+25}, ATTACK_RADIUS, Color{255, 0, 0, 50});  // Полупрозрачный красный круг
        }

        EndDrawing();
    }

    CloseWindow();  // Закрытие окна игры

    return 0;
}
