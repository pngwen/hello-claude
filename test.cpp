#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include "figlet.h"
#include "ofigstream.h"

static constexpr int HEIGHT = 5;
static int g_passed = 0, g_failed = 0;

static std::vector<std::string> split_lines(const std::string& s)
{
    std::vector<std::string> v;
    std::istringstream ss(s);
    std::string line;
    while (std::getline(ss, line))
        v.push_back(line);
    return v;
}

static int count_newlines(const std::string& s)
{
    return (int)std::count(s.begin(), s.end(), '\n');
}

static void section(const std::string& name)
{
    ofigstream out(std::cout);
    out << nofig << name << "\n";
}

static void test(const std::string& name, bool ok)
{
    ofigstream out(std::cout);
    if (ok) {
        out << "PASS" << nofig << (" " + name) << "\n";
        ++g_passed;
    } else {
        out << "FAIL" << nofig << (" " + name) << "\n";
        ++g_failed;
    }
}

int main()
{
    {
        ofigstream hdr(std::cout);
        hdr << "ofigstream\n";
        hdr << nofig << "unit test suite" << "\n";
    }

    // -----------------------------------------------------------------------
    section("[ figlet() ]");

    {
        test("figlet() produces 5 rows for one char",
             count_newlines(figlet("A")) == HEIGHT);
    }
    {
        test("figlet() produces 5 rows for empty string",
             count_newlines(figlet("")) == HEIGHT);
    }
    {
        std::string all;
        for (int c = 32; c <= 126; ++c) all += static_cast<char>(c);
        test("figlet() handles all printable ASCII",
             count_newlines(figlet(all)) == HEIGHT);
    }

    // -----------------------------------------------------------------------
    section("[ ofigstream basic ]");

    {
        std::ostringstream ss;
        ofigstream s(ss);
        s << "A\n";
        test("single char produces 5 rows",
             count_newlines(ss.str()) == HEIGHT);
    }
    {
        std::ostringstream ss;
        ofigstream s(ss);
        s << "Hello\n";
        test("multi-char string produces 5 rows",
             count_newlines(ss.str()) == HEIGHT);
    }
    {
        std::ostringstream ss;
        ofigstream s(ss);
        s << "A\n";
        s << "B\n";
        test("two figlet lines produce 10 rows",
             count_newlines(ss.str()) == 2 * HEIGHT);
    }
    {
        std::ostringstream ss;
        ofigstream s(ss);
        s << "\n";
        test("bare newline produces a single blank line",
             ss.str() == "\n");
    }

    // -----------------------------------------------------------------------
    section("[ nofig alignment ]");

    {
        std::ostringstream ss;
        ofigstream s(ss);
        s << "X" << nofig << "MARKER" << "X\n";
        auto rows = split_lines(ss.str());
        test("default alignment is bottom",
             (int)rows.size() == HEIGHT &&
             rows[HEIGHT - 1].find("MARKER") != std::string::npos &&
             rows[0].find("MARKER") == std::string::npos);
    }
    {
        std::ostringstream ss;
        ofigstream s(ss);
        s << top << "X" << nofig << "MARKER" << "X\n";
        auto rows = split_lines(ss.str());
        test("top puts plain text on row 0",
             (int)rows.size() == HEIGHT &&
             rows[0].find("MARKER") != std::string::npos &&
             rows[HEIGHT - 1].find("MARKER") == std::string::npos);
    }
    {
        std::ostringstream ss;
        ofigstream s(ss);
        s << middle << "X" << nofig << "MARKER" << "X\n";
        auto rows = split_lines(ss.str());
        test("middle puts plain text on row 2",
             (int)rows.size() == HEIGHT &&
             rows[HEIGHT / 2].find("MARKER") != std::string::npos &&
             rows[0].find("MARKER") == std::string::npos);
    }

    // -----------------------------------------------------------------------
    section("[ alignment persistence ]");

    {
        std::ostringstream ss;
        ofigstream s(ss);
        s << top;
        s << "X" << nofig << "FIRST" << "X\n";
        s << "X" << nofig << "SECOND" << "X\n";
        auto rows = split_lines(ss.str());
        test("top alignment persists across two lines",
             (int)rows.size() == 2 * HEIGHT &&
             rows[0].find("FIRST") != std::string::npos &&
             rows[HEIGHT].find("SECOND") != std::string::npos);
    }
    {
        std::ostringstream ss;
        ofigstream s(ss);
        s << middle;
        s << "X" << nofig << "FIRST" << "X\n";
        s << "X" << nofig << "SECOND" << "X\n";
        auto rows = split_lines(ss.str());
        test("middle alignment persists across two lines",
             (int)rows.size() == 2 * HEIGHT &&
             rows[HEIGHT / 2].find("FIRST") != std::string::npos &&
             rows[HEIGHT + HEIGHT / 2].find("SECOND") != std::string::npos);
    }

    // -----------------------------------------------------------------------
    section("[ nofig one-shot ]");

    {
        // "skip" is plain (nofig); "A" should revert to figlet.
        // With bottom alignment, row 0 of the plain segment is blank.
        // If "A" is figlet its glyph has '#' on row 0.
        std::ostringstream ss;
        ofigstream s(ss);
        s << bottom << nofig << "skip" << "A\n";
        auto rows = split_lines(ss.str());
        test("one-shot: field after nofig reverts to figlet",
             (int)rows.size() == HEIGHT &&
             rows[0].find('#') != std::string::npos);
    }
    {
        // Two consecutive nofigs make two plain fields; no figlet '#' on row 0.
        std::ostringstream ss;
        ofigstream s(ss);
        s << bottom << nofig << "skip" << nofig << "A\n";
        auto rows = split_lines(ss.str());
        test("two nofigs make two consecutive plain fields",
             (int)rows.size() == HEIGHT &&
             rows[0].find('#') == std::string::npos);
    }

    // -----------------------------------------------------------------------
    {
        ofigstream sum(std::cout);
        std::string s = std::to_string(g_passed) + " passed, " +
                        std::to_string(g_failed) + " failed";
        sum << nofig << s << "\n";
    }

    return g_failed > 0 ? 1 : 0;
}
