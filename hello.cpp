/**
 * @file hello.cpp
 * @brief Hello, World! rendered through ofigstream.
 *
 * Demonstrates basic ofigstream usage and the nofig manipulator: "Hello"
 * and "World!" appear as 5-row block-character banners while the
 * intervening ", " is rendered as plain text at the default bottom alignment.
 *
 * @author Robert Lowe
 * @author Claude (Anthropic)
 * @copyright MIT License, Copyright (c) 2026 Robert Lowe
 */
#include <iostream>
#include "ofigstream.h"

/// @brief Entry point.
int main()
{
    ofigstream out(std::cout);
    out << "Hello" << nofig << ", " << "World!\n";
    return 0;
}
