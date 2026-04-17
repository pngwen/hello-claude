/**
 * @file ofigstream.h
 * @brief figlet-rendering output stream and supporting types.
 *
 * Provides @c ofigstream, a @c std::ostream subclass that intercepts
 * character output and renders each line through the figlet banner font.
 * Three families of stream manipulators control the rendering:
 *
 *  - @c nofig  — one-shot; the immediately following @c << field is
 *                rendered as normal-sized text.
 *  - @c top / @c middle / @c bottom  — persistent; set the vertical row
 *                at which plain-text (nofig) fields are placed within the
 *                banner height.
 *  - @c fig::red, fig::rainbow, fig::nocolor, etc.  — persistent ANSI
 *                color manipulators; live in the @c fig namespace to avoid
 *                polluting the global namespace with common color names.
 *
 * @author Robert Lowe
 * @author Claude (Anthropic)
 * @copyright MIT License, Copyright (c) 2026 Robert Lowe
 */
#pragma once
#include "figlet.h"
#include <ostream>
#include <streambuf>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// Alignment manipulator types
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
// Color manipulator types
// ---------------------------------------------------------------------------

/// @brief ANSI foreground color values.
enum class Color {
    none,
    red, green, yellow, blue, magenta, cyan, white,
    bright_red, bright_green, bright_yellow, bright_blue,
    bright_magenta, bright_cyan, bright_white
};

/**
 * @brief Tag type for solid-color persistent manipulators.
 *
 * Carries both a foreground color and an optional bold flag so that
 * @c fig::bold (Color::none, bold=true) can be expressed as the same type.
 */
struct color_t {
    Color fg;            ///< Foreground color (Color::none = no change to color).
    bool  bold = false;  ///< Whether to enable bold/bright rendering.
};

/// @brief Tag type for the @c fig::nocolor reset manipulator.
struct nocolor_t {};

/// @brief Tag type for the @c fig::rainbow cycling-color manipulator.
struct rainbow_t {};

/**
 * @brief Namespace for ANSI color and effect manipulators.
 *
 * All color constants are scoped here to avoid conflicts with common
 * identifiers in user code.
 */
namespace fig {
    /// @brief Render subsequent figlet output in red.
    inline constexpr color_t red          {Color::red};
    /// @brief Render subsequent figlet output in green.
    inline constexpr color_t green        {Color::green};
    /// @brief Render subsequent figlet output in yellow.
    inline constexpr color_t yellow       {Color::yellow};
    /// @brief Render subsequent figlet output in blue.
    inline constexpr color_t blue         {Color::blue};
    /// @brief Render subsequent figlet output in magenta.
    inline constexpr color_t magenta      {Color::magenta};
    /// @brief Render subsequent figlet output in cyan.
    inline constexpr color_t cyan         {Color::cyan};
    /// @brief Render subsequent figlet output in white.
    inline constexpr color_t white        {Color::white};
    /// @brief Render subsequent figlet output in bright red.
    inline constexpr color_t bright_red   {Color::bright_red};
    /// @brief Render subsequent figlet output in bright green.
    inline constexpr color_t bright_green {Color::bright_green};
    /// @brief Render subsequent figlet output in bright yellow.
    inline constexpr color_t bright_yellow{Color::bright_yellow};
    /// @brief Render subsequent figlet output in bright blue.
    inline constexpr color_t bright_blue  {Color::bright_blue};
    /// @brief Render subsequent figlet output in bright magenta.
    inline constexpr color_t bright_magenta{Color::bright_magenta};
    /// @brief Render subsequent figlet output in bright cyan.
    inline constexpr color_t bright_cyan  {Color::bright_cyan};
    /// @brief Render subsequent figlet output in bright white.
    inline constexpr color_t bright_white {Color::bright_white};
    /// @brief Enable bold rendering without changing the current color.
    inline constexpr color_t bold         {Color::none, true};
    /// @brief Reset all color, bold, and rainbow state.
    inline constexpr nocolor_t nocolor    {};
    /// @brief Enable rainbow mode: each figlet character cycles through a
    ///        bright six-color palette (red→yellow→green→cyan→blue→magenta).
    inline constexpr rainbow_t rainbow    {};
}

// ---------------------------------------------------------------------------
// figbuf
// ---------------------------------------------------------------------------

/**
 * @brief Stream buffer that renders lines through the figlet font with
 *        optional ANSI color support.
 *
 * Characters are accumulated into typed segments (figlet or plain, each
 * carrying its own color state) until a newline is received, at which point
 * the full line is rendered and forwarded to the wrapped sink stream.
 * The sentinel bytes @c \\x01 / @c \\x02 (written by @c ofigstream) mark
 * the boundaries of plain-text segments.
 *
 * ANSI escape codes are re-applied on every individual row of the 5-row
 * banner because terminal colors do not persist across newlines.
 */
class figbuf : public std::streambuf {
public:
    /// @brief Construct a figbuf that forwards rendered output to @p sink.
    explicit figbuf(std::ostream& sink);

    /// @brief Flush any buffered partial line on destruction.
    ~figbuf();

    /// @brief Set the vertical alignment used when rendering plain segments.
    void set_alignment(Alignment a);

    /// @brief Set a persistent solid foreground color (and bold flag).
    ///        Clears rainbow mode.
    void set_color(Color fg, bool bold);

    /// @brief Reset all color, bold, and rainbow state to defaults.
    void set_nocolor();

    /// @brief Enable rainbow cycling-color mode.  Clears solid color.
    void set_rainbow();

protected:
    /// @brief Handle a single character; recognises sentinel bytes and newline.
    int             overflow(int c) override;

    /// @brief Bulk write; delegates each byte to overflow().
    std::streamsize xsputn(const char* s, std::streamsize n) override;

    /// @brief Flush the sink stream.
    int             sync() override;

private:
    /// @brief One segment of a logical line — either figlet or plain text,
    ///        with its own snapshot of the current color state.
    struct Segment {
        std::string text;
        bool        plain;
        Color       fg      = Color::none; ///< Foreground color at creation time.
        bool        bold    = false;       ///< Bold flag at creation time.
        bool        rainbow = false;       ///< Rainbow mode at creation time.
    };

    std::ostream&        sink_;                       ///< Destination stream.
    Alignment            valign_      = Alignment::bottom; ///< Current alignment.
    Color                cur_fg_      = Color::none;  ///< Current foreground color.
    bool                 cur_bold_    = false;         ///< Current bold state.
    bool                 cur_rainbow_ = false;         ///< Current rainbow state.
    std::vector<Segment> line_;                       ///< Segments for current line.

    /// @brief Ensure the back segment matches @p plain and the current color
    ///        state, creating a new segment when either differs.
    void ensure_segment(bool plain);

    /// @brief Render all segments in @c line_ and write FIGLET_HEIGHT rows to sink_.
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
 * @brief Output stream that renders every line as a figlet banner with
 *        optional ANSI color.
 *
 * Construct with any @c std::ostream as a sink, then use it exactly like
 * @c std::cout.  The @c nofig, @c top / @c middle / @c bottom, and
 * @c fig:: color manipulators control how output is rendered.
 *
 * @note The template @c operator<< must remain defined in this header so
 *       that the compiler can instantiate it at each call site.
 */
class ofigstream : private ofigstream_base, public std::ostream {
public:
    /// @brief Construct an ofigstream that renders output into @p sink.
    explicit ofigstream(std::ostream& sink);

    /// @brief Activate one-shot plain-text mode for the next @c << field.
    ofigstream& operator<<(nofig_t);

    /// @brief Set the persistent vertical alignment for plain-text segments.
    ofigstream& operator<<(align_t a);

    /// @brief Set a persistent solid foreground color (and optional bold).
    ofigstream& operator<<(color_t c);

    /// @brief Reset all color, bold, and rainbow state.
    ofigstream& operator<<(nocolor_t);

    /// @brief Enable rainbow cycling-color mode.
    ofigstream& operator<<(rainbow_t);

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
