#include <iostream>

#include "Valag/utils/Logger.h"
#include "Valag/core/VApp.h"
#include "states/TestingState.h"

int main()
{
    Logger::instance();

    try {
        vlg::VAppCreateInfos createInfos;
        createInfos.name = "VALAG";
        createInfos.xyAngle = 45.0f;
        createInfos.zAngle = 30.0f;

        vlg::VApp app(createInfos);
        app.run(TestingState::instance());
    } catch (const std::exception& e) {
        Logger::fatalError(e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
