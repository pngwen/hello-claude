/**
 * @file ofigstream.cpp
 * @brief Implementation of figbuf, ofigstream_base, and ofigstream.
 *
 * @author Robert Lowe
 * @author Claude (Anthropic)
 * @copyright MIT License, Copyright (c) 2026 Robert Lowe
 */
#include "ofigstream.h"
#include "figlet.h"
#include <sstream>

static constexpr int HEIGHT = 5;

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

/// @brief Ensure the back segment matches @p plain, creating one if needed.
void figbuf::ensure_segment(bool plain)
{
    if (line_.empty() || line_.back().plain != plain)
        line_.push_back({"", plain});
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
        if (line_.empty()) line_.push_back({"", false});
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

/// @brief Render all segments in line_ and write HEIGHT rows to sink_.
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

    // Build a HEIGHT-row grid for each segment.
    std::vector<std::vector<std::string>> all_rows;
    for (auto& seg : line_) {
        std::vector<std::string> rows;

        if (!seg.plain) {
            // Figlet segments: split the rendered output into individual rows.
            std::istringstream ss(figlet(seg.text));
            std::string row;
            while (std::getline(ss, row))
                rows.push_back(row);
            while ((int)rows.size() < HEIGHT) rows.push_back("");
            rows.resize(HEIGHT);

        } else {
            // Plain segments: place the text on the alignment row and pad
            // all other rows with spaces of the same width.
            int ar = HEIGHT - 1;
            if      (valign_ == Alignment::top)    ar = 0;
            else if (valign_ == Alignment::middle) ar = HEIGHT / 2;

            std::string blank(seg.text.size(), ' ');
            for (int r = 0; r < HEIGHT; ++r)
                rows.push_back(r == ar ? seg.text : blank);
        }

        all_rows.push_back(std::move(rows));
    }

    // Write out the assembled grid row by row.
    for (int r = 0; r < HEIGHT; ++r) {
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

/// @brief Forward standard stream manipulators (endl, flush, etc.).
ofigstream& ofigstream::operator<<(std::ostream& (*manip)(std::ostream&))
{
    static_cast<std::ostream&>(*this) << manip;
    return *this;
}
