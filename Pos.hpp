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

    inline bool inside(const Pos &first, const Pos &middle, const Pos &third) {
        return first <= middle && middle <= third;
    }

    inline bool overlaps(const Pos &firstLower, const Pos &firstUpper, const Pos &secondLower, const Pos &secondUpper) {
        // true if secondLower inside first range, secondUpper inside firstRange
        // otherwise, either false or second range contains first range, in which case firstLower inside secondRange
        // where firstRange is {firstLower, firstUpper}, secondRange is {secondLower, secondUpper}
        return inside(firstLower, secondLower, firstUpper) || inside(firstLower, secondUpper, firstUpper) || inside(secondLower, firstLower, secondUpper);
    }
}

#endif //SFML_TEXTBOX_POS_HPP
