#ifndef SFML_TEXTBOX_POS_HPP
#define SFML_TEXTBOX_POS_HPP

#include <iosfwd>

namespace sftb {
    /**
     * An absolute position within a TextBox.
     */
    struct Pos {
        std::size_t line, position;
    };

    inline bool operator<(const Pos &first, const Pos &second) {
        return first.line == second.line ? first.position < second.position : first.line < second.line;
    }

    inline bool operator<=(const Pos &first, const Pos &second) {
        return !(second < first);
    }

    inline bool operator>(const Pos &first, const Pos &second) {
        return second < first;
    }

    inline bool operator>=(const Pos &first, const Pos &second) {
        return !(first < second);
    }

    inline bool operator==(const Pos &first, const Pos &second) {
        return first.line == second.line && first.position == second.position;
    }

    inline bool operator!=(const Pos &first, const Pos &second) {
        return !(first == second);
    }

    inline Pos operator+(const Pos &first, const Pos &second) {
        return {first.line + second.line, first.position + second.position};
    }

    std::ostream &operator<<(std::ostream &out, const Pos &position);
}

#endif //SFML_TEXTBOX_POS_HPP
