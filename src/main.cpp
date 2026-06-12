#include "Core/Game.hpp"
#include <iostream>

int main() {
    TR::Game game;

    if (!game.Initialize()) {
        std::cerr << "Failed to initialize game!\n";
        return 1;
    }

    game.Run();
    game.Shutdown();

    return 0;
}
