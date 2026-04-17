#include "ofigstream.h"
#include "figlet.h"
#include <sstream>

figbuf::figbuf(std::ostream& sink) : sink_(sink) {}

figbuf::~figbuf()
{
    if (!line_.empty())
        flush_line();
}

void figbuf::ensure_segment(bool plain)
{
    if (line_.empty() || line_.back().plain != plain)
        line_.push_back({"", plain});
}

int figbuf::overflow(int c)
{
    if (c == EOF) return EOF;
    switch (c) {
    case '\x01': ensure_segment(true);  break;
    case '\x02': ensure_segment(false); break;
    case '\n':   flush_line(); line_.clear(); break;
    default:
        ensure_segment(false);
        line_.back().text += static_cast<char>(c);
    }
    return c;
}

std::streamsize figbuf::xsputn(const char* s, std::streamsize n)
{
    for (std::streamsize i = 0; i < n; ++i)
        overflow(static_cast<unsigned char>(s[i]));
    return n;
}

int figbuf::sync()
{
    sink_.flush();
    return 0;
}

static constexpr int HEIGHT = 5;

void figbuf::flush_line()
{
    bool has_content = false;
    for (auto& seg : line_)
        if (!seg.text.empty()) { has_content = true; break; }
    if (!has_content) {
        sink_ << '\n';
        return;
    }

    std::vector<std::vector<std::string>> all_rows;
    for (auto& seg : line_) {
        std::vector<std::string> rows;
        if (!seg.plain) {
            std::istringstream ss(figlet(seg.text));
            std::string row;
            while (std::getline(ss, row))
                rows.push_back(row);
            while ((int)rows.size() < HEIGHT) rows.push_back("");
            rows.resize(HEIGHT);
        } else {
            int ar = HEIGHT - 1;
            if      (valign_ == Alignment::top)    ar = 0;
            else if (valign_ == Alignment::middle) ar = HEIGHT / 2;
            std::string blank(seg.text.size(), ' ');
            for (int r = 0; r < HEIGHT; ++r)
                rows.push_back(r == ar ? seg.text : blank);
        }
        all_rows.push_back(std::move(rows));
    }

    for (int r = 0; r < HEIGHT; ++r) {
        for (auto& rows : all_rows)
            sink_ << rows[r];
        sink_ << '\n';
    }
}
