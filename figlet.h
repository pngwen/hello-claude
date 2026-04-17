/**
 * @file figlet.h
 * @brief ASCII block-character banner renderer.
 *
 * Provides a single function that renders a string as a 5-row tall
 * block-character banner composed of '#' glyphs, covering the full
 * printable ASCII range (0x20–0x7E).
 *
 * @author Robert Lowe
 * @author Claude (Anthropic)
 * @copyright MIT License, Copyright (c) 2026 Robert Lowe
 */
#pragma once
#include <string>

/**
 * @brief Render @p text as a 5-row block-character banner.
 *
 * Each printable ASCII character (0x20–0x7E) is mapped to a fixed-width
 * '#'-glyph definition.  Characters outside that range are rendered as
 * spaces of the same default width.  The returned string contains exactly
 * five newline-terminated rows.
 *
 * @param text  The string to render.
 * @return      A multi-line string of five newline-terminated rows.
 */
std::string figlet(const std::string& text);
