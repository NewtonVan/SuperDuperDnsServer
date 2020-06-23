#include <iostream>
#include "application.h"

using namespace dns;

int main(int argc, const char *argv[])
{
    Application *application= new Application();

#ifdef _WIN32
    application->ui(".\\ascii_logo.txt");
#else
    application->ui("./ascii_logo.txt");
#endif

    application->run();

    return 0;
}