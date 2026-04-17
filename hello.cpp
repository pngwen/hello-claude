/**
 * @file hello.cpp
 * @brief Hello, World! rendered through ofigstream with color.
 *
 * Demonstrates ofigstream's rainbow and solid-color manipulators alongside
 * the nofig manipulator.
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

    // Rainbow banner — each character cycles through the bright palette.
    out << fig::rainbow << "Hello, World!\n";

    // Solid color with a plain-text label at middle alignment.
    out << fig::nocolor;
    out << middle << fig::cyan << "C++" << nofig << " in " << fig::bright_yellow << "color!\n";

    return 0;
}
