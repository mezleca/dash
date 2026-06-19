#include "game/game.hpp"
#include <cstdlib>
#include <ctime>

int main() {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    game.initialize();
    return 0;
}
