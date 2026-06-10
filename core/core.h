#pragma once
#include "common.h"

void core_init(GameState* gs);
void core_handle_input(GameState* gs);
void core_update(GameState* gs, float dt);