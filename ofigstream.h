/**
 * @file ofigstream.h
 * @brief figlet-rendering output stream and supporting types.
 *
 * Provides @c ofigstream, a @c std::ostream subclass that intercepts
 * character output and renders each line through the figlet banner font.
 * Two families of stream manipulators allow mixing plain and banner text
 * on the same line:
 *
 *  - @c nofig  — one-shot; the immediately following @c << field is
 *                rendered as normal-sized text.
 *  - @c top / @c middle / @c bottom  — persistent; set the vertical row
 *                at which plain-text fields are placed within the 5-row
 *                banner height.
 *
 * @author Robert Lowe
 * @author Claude (Anthropic)
 * @copyright MIT License, Copyright (c) 2026 Robert Lowe
 */
#pragma once
#include <ostream>
#include <streambuf>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// Manipulator types and constants
// ---------------------------------------------------------------------------

/// @brief Vertical placement of plain-text (nofig) segments within a banner line.
enum class Alignment { top, middle, bottom };

/// @brief Tag type for the @c nofig one-shot manipulator.
struct nofig_t {};

/// @brief Tag type for the persistent vertical-alignment manipulators.
struct align_t {
    Alignment mode; ///< The requested alignment.
};

/// @brief One-shot manipulator: render the next @c << field as plain text.
inline constexpr nofig_t nofig{};

/// @brief Persistent manipulator: place plain-text fields on the top row.
inline constexpr align_t top   {Alignment::top};

/// @brief Persistent manipulator: place plain-text fields on the middle row.
inline constexpr align_t middle{Alignment::middle};

/// @brief Persistent manipulator: place plain-text fields on the bottom row.
inline constexpr align_t bottom{Alignment::bottom};

// ---------------------------------------------------------------------------
// figbuf
// ---------------------------------------------------------------------------

/**
 * @brief Stream buffer that renders lines through the figlet font.
 *
 * Characters are accumulated into typed segments (figlet or plain) until a
 * newline is received, at which point the full line is rendered and forwarded
 * to the wrapped sink stream.  The sentinel bytes @c \\x01 / @c \\x02
 * (written by @c ofigstream) mark the boundaries of plain-text segments.
 */
class figbuf : public std::streambuf {
public:
    /// @brief Construct a figbuf that forwards rendered output to @p sink.
    explicit figbuf(std::ostream& sink);

    /// @brief Flush any buffered partial line on destruction.
    ~figbuf();

    /// @brief Set the vertical alignment used when rendering plain segments.
    void set_alignment(Alignment a);

protected:
    /// @brief Handle a single character; recognises sentinel bytes and newline.
    int             overflow(int c) override;

    /// @brief Bulk write; delegates each byte to overflow().
    std::streamsize xsputn(const char* s, std::streamsize n) override;

    /// @brief Flush the sink stream.
    int             sync() override;

private:
    /// @brief One segment of a logical line — either figlet or plain text.
    struct Segment { std::string text; bool plain; };

    std::ostream&        sink_;                    ///< Destination stream.
    Alignment            valign_ = Alignment::bottom; ///< Current alignment.
    std::vector<Segment> line_;                    ///< Segments for current line.

    /// @brief Ensure the back segment matches @p plain, creating one if needed.
    void ensure_segment(bool plain);

    /// @brief Render all segments in @c line_ and write HEIGHT rows to sink_.
    void flush_line();
};

// ---------------------------------------------------------------------------
// ofigstream_base — initialisation-order helper
// ---------------------------------------------------------------------------

/**
 * @brief Private base that owns the figbuf, ensuring it is constructed before
 *        the std::ostream base which needs a pointer to the buffer.
 */
struct ofigstream_base {
    figbuf buf; ///< The underlying stream buffer.

    /// @brief Construct the base with a sink for figbuf.
    explicit ofigstream_base(std::ostream& sink);
};

// ---------------------------------------------------------------------------
// ofigstream
// ---------------------------------------------------------------------------

/**
 * @brief Output stream that renders every line as a figlet banner.
 *
 * Construct with any @c std::ostream as a sink, then use it exactly like
 * @c std::cout.  The @c nofig, @c top, @c middle, and @c bottom manipulators
 * control how plain text is interspersed with the banner output.
 *
 * @note The template @c operator<< must remain defined in this header so that
 *       the compiler can instantiate it at each call site.
 */
class ofigstream : private ofigstream_base, public std::ostream {
public:
    /// @brief Construct an ofigstream that renders output into @p sink.
    explicit ofigstream(std::ostream& sink);

    /// @brief Activate one-shot plain-text mode for the next @c << field.
    ofigstream& operator<<(nofig_t);

    /// @brief Set the persistent vertical alignment for plain-text segments.
    ofigstream& operator<<(align_t a);

    /// @brief Forward standard stream manipulators (endl, flush, etc.).
    ofigstream& operator<<(std::ostream& (*manip)(std::ostream&));

    /**
     * @brief Write @p val to the stream, wrapping it in plain-text sentinels
     *        if a preceding @c nofig manipulator was applied.
     *
     * When @c nofig_pending_ is set, the value is bracketed with @c \\x01
     * and @c \\x02 sentinel bytes so that figbuf treats it as a plain segment.
     * The flag is cleared immediately, making nofig a true one-shot.
     *
     * @tparam T  Any type supported by @c std::ostream::operator<<.
     * @param  val  The value to write.
     * @return Reference to @c *this for chaining.
     */
    template<typename T>
    ofigstream& operator<<(const T& val)
    {
        if (nofig_pending_) {
            // Bracket this field as a plain-text segment and clear the flag.
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
    bool nofig_pending_ = false; ///< True when the next field should be plain.
};
