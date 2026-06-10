#include "common.h"
#include "core.h"
#include "ui.h"

int main(void) {
    GameState gs = {0};
    core_init(&gs);
    return ui_run(&gs);
}