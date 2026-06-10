#pragma once
#include <windows.h>
#include <stdbool.h>
#include <stdint.h>

#define MAP_W 30
#define MAP_H 20
#define CELL_SIZE 24
#define WIN_W (MAP_W * CELL_SIZE)
#define WIN_H (MAP_H * CELL_SIZE + 60)
#define MAX_ENEMIES 20
#define MAX_ARROWS 10
#define FIXED_DT (1.0 / 30.0)

typedef enum { TILE_WALL = 0, TILE_FLOOR, TILE_STAIRS, TILE_SHOP } TileType;
typedef enum { FOG_HIDDEN = 0, FOG_EXPLORED, FOG_VISIBLE } FogState;

typedef struct {
    int x, y;
    int dir_x, dir_y;
    bool active;
    int timer;
} Entity;

typedef struct {
    TileType map[MAP_H][MAP_W];
    FogState fog[MAP_H][MAP_W];
    
    int px, py;
    int hp, max_hp;
    int gold;
    int level;
    bool game_over;
    
    Entity enemies[MAX_ENEMIES];
    Entity arrows[MAX_ARROWS];
    
    Entity boss;
    int boss_hp;
    int boss_max_hp;
    bool is_boss_floor;
    
    bool keys[256];
    bool running;
    int last_dir_x;
    int last_dir_y;
    
    bool shop_open;
    int shop_cursor;
    int shop_x, shop_y;
    
    int move_cd_max;
    int move_cd_curr;
    int shoot_cd_max;
    int shoot_cooldown;
    int fov_radius;
    
    int cost_fire_rate;
    int cost_move_speed;
    int cost_fov;
} GameState;