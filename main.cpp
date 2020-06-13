#include <iostream>
#include "application.h"

using namespace dns;

int main(int argc, const char *argv[])
{
    Application *application= new Application();
    application->run();

    return 0;
}