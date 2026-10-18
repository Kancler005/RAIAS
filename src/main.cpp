#include "engine/Application.h"

int main(
    int argc,
    char* argv[]
)
{
    (void)argc;
    (void)argv;

    Application application;

    if (!application.Initialize())
    {
        application.Shutdown();

        return 1;
    }

    application.Run();

    application.Shutdown();

    return 0;
}