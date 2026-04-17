#include <iostream>
#include "ofigstream.h"

int main()
{
    ofigstream out(std::cout);
    out << "Hello" << nofig << ", " << "World!\n";
    return 0;
}
