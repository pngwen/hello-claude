/**
 * @file ofigstream.cpp
 * @brief Implementation of figbuf, ofigstream_base, and ofigstream.
 *
 * @author Robert Lowe
 * @author Claude (Anthropic)
 * @copyright MIT License, Copyright (c) 2026 Robert Lowe
 */
#include "ofigstream.h"
#include <sstream>

// ---------------------------------------------------------------------------
// ANSI helpers
// ---------------------------------------------------------------------------

/// @brief Map a Color enum value to its ANSI SGR integer code.
static int ansi_code(Color fg)
{
    switch (fg) {
    case Color::red:            return 31;
    case Color::green:          return 32;
    case Color::yellow:         return 33;
    case Color::blue:           return 34;
    case Color::magenta:        return 35;
    case Color::cyan:           return 36;
    case Color::white:          return 37;
    case Color::bright_red:     return 91;
    case Color::bright_green:   return 92;
    case Color::bright_yellow:  return 93;
    case Color::bright_blue:    return 94;
    case Color::bright_magenta: return 95;
    case Color::bright_cyan:    return 96;
    case Color::bright_white:   return 97;
    default:                    return 0;
    }
}

/**
 * @brief Wrap @p s with ANSI SGR open/reset codes for one terminal row.
 *
 * ANSI colors do not persist across newlines, so every row of the 5-row
 * banner must carry its own escape sequence.  Returns @p s unchanged when
 * neither @p fg nor @p bold is active.
 *
 * @param s     The row string to wrap.
 * @param fg    Foreground color (Color::none = no color code emitted).
 * @param bold  Whether to prepend the bold (@c \\033[1m) code.
 * @return      The wrapped string, or @p s unmodified if no codes apply.
 */
static std::string ansi_wrap(const std::string& s, Color fg, bool bold)
{
    if (fg == Color::none && !bold) return s;

    // Build the opening sequence, then the content, then the reset.
    std::string out;
    if (bold)              out += "\033[1m";
    if (fg != Color::none) out += "\033[" + std::to_string(ansi_code(fg)) + "m";
    out += s;
    out += "\033[0m";

    return out;
}

// ---------------------------------------------------------------------------
// figbuf
// ---------------------------------------------------------------------------

/// @brief Construct a figbuf that forwards rendered output to @p sink.
figbuf::figbuf(std::ostream& sink) : sink_(sink) {}

/// @brief Flush any buffered partial line on destruction.
figbuf::~figbuf()
{
    if (!line_.empty())
        flush_line();
}

/// @brief Set the vertical alignment used when rendering plain segments.
void figbuf::set_alignment(Alignment a)
{
    valign_ = a;
}

/// @brief Set a persistent solid foreground color (and bold flag).
void figbuf::set_color(Color fg, bool bold)
{
    cur_fg_      = fg;
    cur_bold_    = bold;
    cur_rainbow_ = false;   // solid color overrides rainbow
}

/// @brief Reset all color, bold, and rainbow state to defaults.
void figbuf::set_nocolor()
{
    cur_fg_      = Color::none;
    cur_bold_    = false;
    cur_rainbow_ = false;
}

/// @brief Enable rainbow cycling-color mode.  Clears solid color.
void figbuf::set_rainbow()
{
    cur_rainbow_ = true;
    cur_fg_      = Color::none;   // rainbow overrides solid color
    cur_bold_    = false;
}

/// @brief Ensure the back segment matches @p plain and current color state.
void figbuf::ensure_segment(bool plain)
{
    // Start a new segment whenever the type or any color attribute changes.
    bool needs_new = line_.empty()
                  || line_.back().plain   != plain
                  || line_.back().fg      != cur_fg_
                  || line_.back().bold    != cur_bold_
                  || line_.back().rainbow != cur_rainbow_;

    if (needs_new)
        line_.push_back({"", plain, cur_fg_, cur_bold_, cur_rainbow_});
}

/// @brief Handle a single character; recognises sentinel bytes and newline.
int figbuf::overflow(int c)
{
    if (c == EOF) return EOF;

    // Sentinel \x01 / \x02 mark the start of a plain or figlet segment.
    // \n flushes the completed line; all other bytes append to the current segment.
    switch (c) {
    case '\x01': ensure_segment(true);  break;
    case '\x02': ensure_segment(false); break;
    case '\n':   flush_line(); line_.clear(); break;
    default:
        if (line_.empty()) line_.push_back({"", false, cur_fg_, cur_bold_, cur_rainbow_});
        line_.back().text += static_cast<char>(c);
    }

    return c;
}

/// @brief Bulk write; delegates each byte to overflow().
std::streamsize figbuf::xsputn(const char* s, std::streamsize n)
{
    for (std::streamsize i = 0; i < n; ++i)
        overflow(static_cast<unsigned char>(s[i]));
    return n;
}

/// @brief Flush the sink stream.
int figbuf::sync()
{
    sink_.flush();
    return 0;
}

/// @brief Render all segments in line_ and write FIGLET_HEIGHT rows to sink_.
void figbuf::flush_line()
{
    // A line with no non-empty segments is just a blank line.
    bool has_content = false;
    for (auto& seg : line_)
        if (!seg.text.empty()) { has_content = true; break; }

    if (!has_content) {
        sink_ << '\n';
        return;
    }

    // Rainbow palette: six bright colors cycling per figlet character.
    static const Color PALETTE[] = {
        Color::bright_red,   Color::bright_yellow, Color::bright_green,
        Color::bright_cyan,  Color::bright_blue,   Color::bright_magenta
    };

    // Build a FIGLET_HEIGHT-row grid for each segment.
    std::vector<std::vector<std::string>> all_rows;
    for (auto& seg : line_) {
        std::vector<std::string> rows;

        if (!seg.plain && seg.rainbow) {
            // Rainbow figlet: render each character independently and wrap
            // each glyph row with its own cycling-palette color code.
            rows.assign(FIGLET_HEIGHT, "");
            for (size_t i = 0; i < seg.text.size(); ++i) {
                Color c    = PALETTE[i % 6];
                auto glyph = figlet_char(seg.text[i]);
                for (int r = 0; r < FIGLET_HEIGHT; ++r) {
                    if (i > 0) rows[r] += ' ';
                    rows[r] += ansi_wrap(glyph[r], c, seg.bold);
                }
            }

        } else if (!seg.plain) {
            // Standard figlet rendering: split output into rows, then apply
            // color / bold to each row individually.
            std::istringstream ss(figlet(seg.text));
            std::string row;
            while (std::getline(ss, row))
                rows.push_back(row);
            while ((int)rows.size() < FIGLET_HEIGHT) rows.push_back("");
            rows.resize(FIGLET_HEIGHT);

            if (seg.fg != Color::none || seg.bold)
                for (auto& r : rows)
                    r = ansi_wrap(r, seg.fg, seg.bold);

        } else {
            // Plain text: place the content on the alignment row and pad all
            // other rows with spaces of the same width, then apply color.
            int ar = FIGLET_HEIGHT - 1;
            if      (valign_ == Alignment::top)    ar = 0;
            else if (valign_ == Alignment::middle) ar = FIGLET_HEIGHT / 2;

            std::string blank(seg.text.size(), ' ');
            for (int r = 0; r < FIGLET_HEIGHT; ++r) {
                std::string content = (r == ar ? seg.text : blank);
                rows.push_back(ansi_wrap(content, seg.fg, seg.bold));
            }
        }

        all_rows.push_back(std::move(rows));
    }

    // Write out the assembled grid row by row.
    for (int r = 0; r < FIGLET_HEIGHT; ++r) {
        for (auto& rows : all_rows)
            sink_ << rows[r];
        sink_ << '\n';
    }
}

// ---------------------------------------------------------------------------
// ofigstream_base
// ---------------------------------------------------------------------------

/// @brief Construct the base with a sink for figbuf.
ofigstream_base::ofigstream_base(std::ostream& sink) : buf(sink) {}

// ---------------------------------------------------------------------------
// ofigstream
// ---------------------------------------------------------------------------

/// @brief Construct an ofigstream that renders output into @p sink.
ofigstream::ofigstream(std::ostream& sink)
    : ofigstream_base(sink), std::ostream(&buf)
{}

/// @brief Activate one-shot plain-text mode for the next << field.
ofigstream& ofigstream::operator<<(nofig_t)
{
    nofig_pending_ = true;
    return *this;
}

/// @brief Set the persistent vertical alignment for plain-text segments.
ofigstream& ofigstream::operator<<(align_t a)
{
    buf.set_alignment(a.mode);
    return *this;
}

/// @brief Set a persistent solid foreground color (and optional bold).
ofigstream& ofigstream::operator<<(color_t c)
{
    buf.set_color(c.fg, c.bold);
    return *this;
}

/// @brief Reset all color, bold, and rainbow state.
ofigstream& ofigstream::operator<<(nocolor_t)
{
    buf.set_nocolor();
    return *this;
}

/// @brief Enable rainbow cycling-color mode.
ofigstream& ofigstream::operator<<(rainbow_t)
{
    buf.set_rainbow();
    return *this;
}

/// @brief Forward standard stream manipulators (endl, flush, etc.).
ofigstream& ofigstream::operator<<(std::ostream& (*manip)(std::ostream&))
{
    static_cast<std::ostream&>(*this) << manip;
    return *this;
}
