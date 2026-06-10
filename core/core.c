#include "core.h"
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define MAX_ROOMS 10
#define ENEMY_VISION_RADIUS 5

typedef struct {
    int x, y, w, h;
    int center_x, center_y;
} Room;

static Room rooms[MAX_ROOMS];
static int room_count = 0;

static void fill_map(GameState* gs) {
    for (int y = 0; y < MAP_H; ++y)
        for (int x = 0; x < MAP_W; ++x)
            gs->map[y][x] = TILE_WALL;
}

static void create_room(GameState* gs, Room* r) {
    for (int y = r->y; y < r->y + r->h; ++y)
        for (int x = r->x; x < r->x + r->w; ++x)
            if (y > 0 && y < MAP_H - 1 && x > 0 && x < MAP_W - 1)
                gs->map[y][x] = TILE_FLOOR;
    r->center_x = r->x + r->w / 2;
    r->center_y = r->y + r->h / 2;
}

static bool rooms_intersect(Room* a, Room* b) {
    return !(a->x + a->w < b->x || b->x + b->w < a->x || 
             a->y + a->h < b->y || b->y + b->h < a->y);
}

static void create_h_tunnel(GameState* gs, int x1, int x2, int y) {
    for (int x = (x1 < x2 ? x1 : x2); x <= (x1 < x2 ? x2 : x1); ++x)
        if (y > 0 && y < MAP_H - 1 && x > 0 && x < MAP_W - 1) 
            gs->map[y][x] = TILE_FLOOR;
}

static void create_v_tunnel(GameState* gs, int y1, int y2, int x) {
    for (int y = (y1 < y2 ? y1 : y2); y <= (y1 < y2 ? y2 : y1); ++y)
        if (y > 0 && y < MAP_H - 1 && x > 0 && x < MAP_W - 1) 
            gs->map[y][x] = TILE_FLOOR;
}

static void connect_rooms(GameState* gs, Room* a, Room* b) {
    if (rand() % 2 == 0) { 
        create_h_tunnel(gs, a->center_x, b->center_x, a->center_y); 
        create_v_tunnel(gs, a->center_y, b->center_y, b->center_x); 
    } else { 
        create_v_tunnel(gs, a->center_y, b->center_y, a->center_x); 
        create_h_tunnel(gs, a->center_x, b->center_x, b->center_y); 
    }
}

static void generate_dungeon(GameState* gs) {
    fill_map(gs); 
    room_count = 0; 
    int att = 0;
    
    while (room_count < MAX_ROOMS && att < 100) {
        att++;
        Room nr = {2 + rand() % (MAP_W - 7), 2 + rand() % (MAP_H - 7), 
                   4 + rand() % 5, 4 + rand() % 5, 0, 0};
        bool ov = false;
        for (int i = 0; i < room_count; ++i) {
            if (rooms_intersect(&nr, &rooms[i])) { 
                ov = true; 
                break; 
            }
        }
        if (!ov) { 
            create_room(gs, &nr); 
            if (room_count > 0) connect_rooms(gs, &rooms[room_count - 1], &nr); 
            rooms[room_count++] = nr; 
        }
    }
    
    if (room_count < 2) { 
        room_count = 0; 
        Room big = {3, 3, MAP_W - 6, MAP_H - 8, (MAP_W - 6) / 2, (MAP_H - 8) / 2}; 
        create_room(gs, &big); 
        rooms[room_count++] = big; 
    }
    
    gs->px = rooms[0].center_x; 
    gs->py = rooms[0].center_y;
    gs->last_dir_x = 1;
    gs->last_dir_y = 0;
    
    Room* last = &rooms[room_count - 1];
    gs->map[last->y + 1 + rand() % (last->h - 2)][last->x + 1 + rand() % (last->w - 2)] = TILE_STAIRS;
    
    if (rand() % 3 == 0 && room_count > 2) {
        int r_idx = 1 + rand() % (room_count - 2); 
        Room* sr = &rooms[r_idx];
        int sx = sr->x + 1 + rand() % (sr->w - 2), sy = sr->y + 1 + rand() % (sr->h - 2);
        if (gs->map[sy][sx] == TILE_FLOOR) gs->map[sy][sx] = TILE_SHOP;
    }
    
    gs->is_boss_floor = (gs->level % 10 == 0);
    if (gs->is_boss_floor) {
        int r_idx = 1 + rand() % (room_count > 1 ? room_count - 1 : 1);
        Room* br = &rooms[r_idx];
        gs->boss.x = br->x + 1 + rand() % (br->w - 2);
        gs->boss.y = br->y + 1 + rand() % (br->h - 2);
        gs->boss.active = true;
        gs->boss.timer = 0;
        
        gs->boss_max_hp = 5;
        for (int i = 10; i < gs->level; i += 10) gs->boss_max_hp *= 2;
        gs->boss_hp = gs->boss_max_hp;
    } else {
        gs->boss.active = false;
    }
}

static void spawn_enemies(GameState* gs) {
    for (int i = 0; i < MAX_ENEMIES; ++i) gs->enemies[i].active = false;
    for (int i = 0; i < MAX_ARROWS; ++i) gs->arrows[i].active = false;
    
    int cnt = 0, att = 0;
    int max_enemies = gs->is_boss_floor ? 2 : 4;
    
    while (cnt < max_enemies && att < 200) {
        att++;
        int ex = rand() % MAP_W, ey = rand() % MAP_H;
        if (gs->map[ey][ex] != TILE_FLOOR) continue;
        if (abs(ex - gs->px) + abs(ey - gs->py) < 6) continue;
        if (gs->is_boss_floor && gs->boss.active && ex == gs->boss.x && ey == gs->boss.y) continue;
        
        bool occ = false;
        for (int i = 0; i < cnt; ++i) {
            if (gs->enemies[i].active && gs->enemies[i].x == ex && gs->enemies[i].y == ey) { 
                occ = true; 
                break; 
            }
        }
        if (!occ) {
            gs->enemies[cnt].x = ex; 
            gs->enemies[cnt].y = ey;
            gs->enemies[cnt].active = true; 
            gs->enemies[cnt].timer = 0; 
            cnt++;
        }
    }
}

static void update_fog(GameState* gs) {
    for (int y = 0; y < MAP_H; ++y) 
        for (int x = 0; x < MAP_W; ++x)
            if (gs->fog[y][x] == FOG_VISIBLE) gs->fog[y][x] = FOG_EXPLORED;
            
    for (int y = 0; y < MAP_H; ++y) 
        for (int x = 0; x < MAP_W; ++x)
            if ((x - gs->px) * (x - gs->px) + (y - gs->py) * (y - gs->py) <= gs->fov_radius * gs->fov_radius)
                gs->fog[y][x] = FOG_VISIBLE;
}

static void try_shoot(GameState* gs) {
    if (gs->shoot_cooldown > 0) return;
    for (int i = 0; i < MAX_ARROWS; ++i) {
        if (!gs->arrows[i].active) {
            gs->arrows[i].active = true;
            gs->arrows[i].x = gs->px; 
            gs->arrows[i].y = gs->py;
            gs->arrows[i].dir_x = gs->last_dir_x; 
            gs->arrows[i].dir_y = gs->last_dir_y;
            gs->shoot_cooldown = gs->shoot_cd_max;
            return;
        }
    }
}

void core_init(GameState* gs) {
    srand((unsigned)time(NULL));
    
    gs->max_hp = 10; gs->hp = 10; gs->gold = 0; gs->level = 1;
    gs->game_over = false; gs->shop_open = false; gs->shop_cursor = 0;
    gs->move_cd_max = 2; gs->move_cd_curr = 0;
    gs->shoot_cd_max = 15; gs->shoot_cooldown = 0;
    gs->fov_radius = 6;
    gs->cost_fire_rate = 5; gs->cost_move_speed = 5; gs->cost_fov = 8;
    gs->boss.active = false; gs->is_boss_floor = false;
    
    for (int y = 0; y < MAP_H; ++y) 
        for (int x = 0; x < MAP_W; ++x) gs->fog[y][x] = FOG_HIDDEN;
        
    generate_dungeon(gs); 
    spawn_enemies(gs); 
    update_fog(gs);
}

void core_handle_input(GameState* gs) {
    if (gs->game_over) {
        if (gs->keys['R'] || gs->keys[VK_RETURN]) core_init(gs);
        gs->keys['R'] = false; gs->keys[VK_RETURN] = false;
        return;
    }
    
    if (gs->shop_open) {
        if (gs->keys[VK_UP]) { gs->shop_cursor--; if (gs->shop_cursor < 0) gs->shop_cursor = 2; gs->keys[VK_UP] = false; }
        if (gs->keys[VK_DOWN]) { gs->shop_cursor++; if (gs->shop_cursor > 2) gs->shop_cursor = 0; gs->keys[VK_DOWN] = false; }
        if (gs->keys[VK_RETURN] || gs->keys[VK_SPACE]) {
            bool bought = false;
            if (gs->shop_cursor == 0 && gs->gold >= gs->cost_fire_rate) { 
                gs->gold -= gs->cost_fire_rate; 
                gs->shoot_cd_max = (gs->shoot_cd_max > 5) ? gs->shoot_cd_max - 3 : 5; 
                gs->cost_fire_rate += 3; 
                bought = true; 
            } else if (gs->shop_cursor == 1 && gs->gold >= gs->cost_move_speed) { 
                gs->gold -= gs->cost_move_speed; 
                gs->move_cd_max = (gs->move_cd_max > 0) ? gs->move_cd_max - 1 : 0; 
                gs->cost_move_speed += 3; 
                bought = true; 
            } else if (gs->shop_cursor == 2 && gs->gold >= gs->cost_fov) { 
                gs->gold -= gs->cost_fov; 
                gs->fov_radius++; 
                gs->cost_fov += 4; 
                bought = true; 
            }
            if (bought) { gs->shop_open = false; update_fog(gs); }
            gs->keys[VK_RETURN] = false; gs->keys[VK_SPACE] = false;
        }
        if (gs->keys['Q'] || gs->keys['q']) { gs->shop_open = false; gs->keys['Q'] = false; gs->keys['q'] = false; }
        return;
    }
    
    if (gs->keys[VK_SPACE]) { 
        try_shoot(gs); 
        gs->keys[VK_SPACE] = false; 
    }

    if (gs->move_cd_curr > 0) {
    } else {
        int nx = gs->px, ny = gs->py;
        bool want_move = false;
        int dir_x = 0, dir_y = 0;

        if (gs->keys[VK_LEFT] || gs->keys['A']) { nx--; dir_x = -1; dir_y = 0; want_move = true; }
        else if (gs->keys[VK_RIGHT] || gs->keys['D']) { nx++; dir_x = 1; dir_y = 0; want_move = true; }
        else if (gs->keys[VK_UP] || gs->keys['W']) { ny--; dir_x = 0; dir_y = -1; want_move = true; }
        else if (gs->keys[VK_DOWN] || gs->keys['S']) { ny++; dir_x = 0; dir_y = 1; want_move = true; }

        if (want_move) {
            gs->last_dir_x = dir_x;
            gs->last_dir_y = dir_y;

            if (nx >= 0 && nx < MAP_W && ny >= 0 && ny < MAP_H && gs->map[ny][nx] != TILE_WALL) {
                bool blocked = false;

                for (int i = 0; i < MAX_ENEMIES; ++i) {
                    if (gs->enemies[i].active && gs->enemies[i].x == nx && gs->enemies[i].y == ny) {
                        blocked = true;
                        if (rand() % 2 == 0) gs->hp--;
                        if (gs->hp <= 0) { gs->hp = 0; gs->game_over = true; }
                        break;
                    }
                }

                if (!blocked && gs->is_boss_floor && gs->boss.active && gs->boss.x == nx && gs->boss.y == ny) {
                    blocked = true;
                    gs->hp--;
                    if (gs->hp <= 0) { gs->hp = 0; gs->game_over = true; }
                }

                if (!blocked && !gs->game_over) {
                    gs->px = nx;
                    gs->py = ny;
                    gs->move_cd_curr = gs->move_cd_max;

                    if (gs->map[ny][nx] == TILE_STAIRS) {
                        gs->level++; 
                        gs->max_hp += 2; 
                        gs->hp = gs->max_hp; 
                        gs->gold += 5;
                        for (int y = 0; y < MAP_H; ++y)
                            for (int x = 0; x < MAP_W; ++x) gs->fog[y][x] = FOG_HIDDEN;
                        generate_dungeon(gs); 
                        spawn_enemies(gs);
                    } else if (gs->map[ny][nx] == TILE_SHOP) {
                        gs->shop_open = true; 
                        gs->shop_cursor = 0; 
                        gs->map[ny][nx] = TILE_FLOOR;
                    }
                }
            }
        }
    }
}

void core_update(GameState* gs, float dt) {
    (void)dt;
    if (gs->game_over) return;
    if (gs->shop_open) { update_fog(gs); return; }
    
    if (gs->shoot_cooldown > 0) gs->shoot_cooldown--;
    if (gs->move_cd_curr > 0) gs->move_cd_curr--;

    for (int i = 0; i < MAX_ARROWS; ++i) {
        if (!gs->arrows[i].active) continue;
        int nx = gs->arrows[i].x + gs->arrows[i].dir_x, ny = gs->arrows[i].y + gs->arrows[i].dir_y;
        
        if (nx < 0 || nx >= MAP_W || ny < 0 || ny >= MAP_H || gs->map[ny][nx] == TILE_WALL) { 
            gs->arrows[i].active = false; 
            continue; 
        }
        
        bool hit = false;
        for (int j = 0; j < MAX_ENEMIES; ++j) {
            if (gs->enemies[j].active && gs->enemies[j].x == nx && gs->enemies[j].y == ny) {
                gs->enemies[j].active = false; 
                gs->gold += (rand() % 3) + 1; 
                gs->arrows[i].active = false; 
                hit = true; 
                break;
            }
        }
        if (!hit && gs->is_boss_floor && gs->boss.active && gs->boss.x == nx && gs->boss.y == ny) {
            gs->boss_hp--;
            if (gs->boss_hp <= 0) {
                gs->boss.active = false;
                gs->gold += 20 + (gs->level / 10) * 5;
            }
            gs->arrows[i].active = false;
            hit = true;
        }
        if (!hit) { 
            gs->arrows[i].x = nx; 
            gs->arrows[i].y = ny; 
        }
    }
    
    for (int i = 0; i < MAX_ENEMIES; ++i) {
        if (!gs->enemies[i].active) continue;
        gs->enemies[i].timer++;
        if (gs->enemies[i].timer < 10) continue;
        gs->enemies[i].timer = 0;
        
        int dist = abs(gs->px - gs->enemies[i].x) + abs(gs->py - gs->enemies[i].y);
        if (dist > ENEMY_VISION_RADIUS) continue;
        
        int dx = 0, dy = 0;
        if (gs->px > gs->enemies[i].x) dx = 1; else if (gs->px < gs->enemies[i].x) dx = -1;
        if (gs->py > gs->enemies[i].y) dy = 1; else if (gs->py < gs->enemies[i].y) dy = -1;
        
        int nx = gs->enemies[i].x + dx, ny = gs->enemies[i].y + dy;
        bool can = !(nx < 0 || nx >= MAP_W || ny < 0 || ny >= MAP_H || gs->map[ny][nx] == TILE_WALL);
        
        if (can) {
            for (int j = 0; j < MAX_ENEMIES; ++j) {
                if (i != j && gs->enemies[j].active && gs->enemies[j].x == nx && gs->enemies[j].y == ny) { 
                    can = false; 
                    break; 
                }
            }
        }
        if (can && nx == gs->px && ny == gs->py) { 
            gs->hp--; 
            if (gs->hp <= 0) { gs->hp = 0; gs->game_over = true; } 
            can = false; 
        }
        if (can) { 
            gs->enemies[i].x = nx; 
            gs->enemies[i].y = ny; 
        }
    }
    
    if (gs->is_boss_floor && gs->boss.active) {
        gs->boss.timer++;
        if (gs->boss.timer < 4) { } else {
            gs->boss.timer = 0;
            int dist = abs(gs->px - gs->boss.x) + abs(gs->py - gs->boss.y);
            if (dist <= 12) {
                int dx = 0, dy = 0;
                if (gs->px > gs->boss.x) dx = 1; else if (gs->px < gs->boss.x) dx = -1;
                if (gs->py > gs->boss.y) dy = 1; else if (gs->py < gs->boss.y) dy = -1;
                
                int nx = gs->boss.x + dx, ny = gs->boss.y + dy;
                bool can = !(nx < 0 || nx >= MAP_W || ny < 0 || ny >= MAP_H || gs->map[ny][nx] == TILE_WALL);
                
                if (can && nx == gs->px && ny == gs->py) { 
                    gs->hp--; 
                    if (gs->hp <= 0) { gs->hp = 0; gs->game_over = true; } 
                    can = false; 
                }
                if (can) { 
                    gs->boss.x = nx; 
                    gs->boss.y = ny; 
                }
            }
        }
    }
    update_fog(gs);
}