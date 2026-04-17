/**
 * @file hello.cpp
 * @brief Animated shimmering "Hello, World!" banner with alignment cycling.
 *
 * Renders "Hello, World!" as a figlet banner with per-character rainbow colors
 * that shift each frame to produce a shimmering marching effect.  A pair of
 * nofig labels ("hello" and "world") flank the banner and cycle through top,
 * middle, and bottom vertical alignments every fifteen frames.
 *
 * The program loops indefinitely at ~10 fps until SIGINT or SIGTERM arrives,
 * at which point it restores the terminal (cursor visible, colors reset, screen
 * cleared) before exiting.
 *
 * @author Robert Lowe
 * @author Claude (Anthropic)
 * @copyright MIT License, Copyright (c) 2026 Robert Lowe
 */
#include <iostream>
#include <csignal>
#include <unistd.h>
#include "ofigstream.h"

static constexpr const char CLEAR[]     = "\033[2J";
static constexpr const char HOME[]      = "\033[H";
static constexpr const char HIDE_CUR[]  = "\033[?25l";
static constexpr const char SHOW_CUR[]  = "\033[?25h";
static constexpr const char RESET_CLR[] = "\033[0m";

static volatile sig_atomic_t g_running = 1;

/// @brief Signal handler: clear the run flag so the main loop exits cleanly.
static void handle_signal(int) { g_running = 0; }

/// @brief Restore the terminal to a sane state on exit.
static void reset_terminal()
{
    std::cout << RESET_CLR << SHOW_CUR << CLEAR << HOME;
    std::cout.flush();
}

/// @brief Entry point — runs the animation loop.
int main()
{
    std::signal(SIGINT,  handle_signal);
    std::signal(SIGTERM, handle_signal);

    // Hide the cursor and clear the screen once before the loop begins.
    std::cout << HIDE_CUR << CLEAR;
    std::cout.flush();

    // Six-color palette — matches the built-in fig::rainbow order so the
    // manual shimmer looks consistent with the library's rainbow mode.
    static const color_t PALETTE[] = {
        fig::bright_red,   fig::bright_yellow, fig::bright_green,
        fig::bright_cyan,  fig::bright_blue,   fig::bright_magenta
    };
    static constexpr const char TEXT[] = "Hello, World!";

    // Three alignment modes to cycle through every 15 frames.
    static const align_t ALIGNS[] = {
        align_t{Alignment::top},
        align_t{Alignment::middle},
        align_t{Alignment::bottom}
    };

    ofigstream out(std::cout);
    int frame = 0;

    while (g_running) {
        // Return to the top-left corner without clearing so the banner
        // overwrites in place, avoiding flicker.
        std::cout << HOME;

        // Update alignment and print the leading plain-text label.
        out << ALIGNS[(frame / 15) % 3];
        out << nofig << "hello ";

        // Shimmer: shift the palette start by one step per frame so the
        // colors appear to march from left to right.
        int offset = frame % 6;
        for (int i = 0; TEXT[i] != '\0'; ++i)
            out << PALETTE[(i + offset) % 6] << TEXT[i];

        // Trailing label and newline flush the assembled line.
        out << fig::nocolor << nofig << " world\n";

        std::cout.flush();
        usleep(100000);   // 100 ms per frame ≈ 10 fps
        ++frame;
    }

    reset_terminal();
    return 0;
}
