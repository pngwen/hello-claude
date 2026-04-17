/**
 * @file figlet.h
 * @brief ASCII block-character banner renderer.
 *
 * Provides functions that render text as a 5-row tall block-character
 * banner composed of '#' glyphs, covering the full printable ASCII
 * range (0x20–0x7E).
 *
 * @author Robert Lowe
 * @author Claude (Anthropic)
 * @copyright MIT License, Copyright (c) 2026 Robert Lowe
 */
#pragma once
#include <array>
#include <string>

/// Height of every figlet banner in rows.
static constexpr int FIGLET_HEIGHT = 5;

/**
 * @brief Return the @c FIGLET_HEIGHT glyph rows for a single character.
 *
 * Printable ASCII characters (0x20–0x7E) are mapped to their '#'-glyph
 * definition.  Any other byte yields a row of three spaces.  The returned
 * strings have consistent width for the given character but widths differ
 * between characters.
 *
 * @param c  The character to render.
 * @return   Array of FIGLET_HEIGHT row strings, one per banner row.
 */
std::array<std::string, FIGLET_HEIGHT> figlet_char(char c);

/**
 * @brief Render @p text as a multi-line block-character banner.
 *
 * Each character in @p text is rendered via figlet_char() and the glyphs
 * are joined with single-space separators.  The returned string contains
 * exactly FIGLET_HEIGHT newline-terminated rows.
 *
 * @param text  The string to render.
 * @return      A multi-line string of FIGLET_HEIGHT newline-terminated rows.
 */
std::string figlet(const std::string& text);
