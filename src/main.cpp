#include "game/game.hpp"
#include <ctime>

int main() {
    std::srand(std::time(0));

    g_game.initialize();
    return 0;
}
