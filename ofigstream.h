#pragma once
#include <ostream>
#include <streambuf>
#include <string>
#include <vector>

enum class Alignment { top, middle, bottom };

struct nofig_t {};
struct align_t { Alignment mode; };

inline constexpr nofig_t nofig{};
inline constexpr align_t top   {Alignment::top};
inline constexpr align_t middle{Alignment::middle};
inline constexpr align_t bottom{Alignment::bottom};

class figbuf : public std::streambuf {
public:
    explicit figbuf(std::ostream& sink);
    ~figbuf();
    void set_alignment(Alignment a) { valign_ = a; }

protected:
    int             overflow(int c) override;
    std::streamsize xsputn(const char* s, std::streamsize n) override;
    int             sync() override;

private:
    struct Segment { std::string text; bool plain; };

    std::ostream&        sink_;
    Alignment            valign_ = Alignment::bottom;
    std::vector<Segment> line_;

    void ensure_segment(bool plain);
    void flush_line();
};

struct ofigstream_base {
    figbuf buf;
    explicit ofigstream_base(std::ostream& sink) : buf(sink) {}
};

class ofigstream : private ofigstream_base, public std::ostream {
public:
    explicit ofigstream(std::ostream& sink)
        : ofigstream_base(sink), std::ostream(&buf) {}

    ofigstream& operator<<(nofig_t) {
        nofig_pending_ = true;
        return *this;
    }

    ofigstream& operator<<(align_t a) {
        buf.set_alignment(a.mode);
        return *this;
    }

    // Handles std::endl, std::flush, std::ends, etc.
    ofigstream& operator<<(std::ostream& (*manip)(std::ostream&)) {
        static_cast<std::ostream&>(*this) << manip;
        return *this;
    }

    template<typename T>
    ofigstream& operator<<(const T& val) {
        if (nofig_pending_) {
            nofig_pending_ = false;
            put('\x01');
            static_cast<std::ostream&>(*this) << val;
            put('\x02');
        } else {
            static_cast<std::ostream&>(*this) << val;
        }
        return *this;
    }

private:
    bool nofig_pending_ = false;
};
