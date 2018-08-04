#include <iostream>

#include "Valag/utils/Logger.h"
#include "Valag/core/VApp.h"
#include "states/TestingState.h"

int main()
{
    Logger::instance();

    vlg::VApp app("VALAG");

    try {
        app.run(TestingState::instance());
    } catch (const std::exception& e) {
        Logger::fatalError(e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
