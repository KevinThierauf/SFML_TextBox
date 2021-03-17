#include "CharPos.hpp"
#include "TextBox.hpp"

namespace sftb::detail {
    void CharPosData::reduceRelative() const {
        Relative &relative = getRelative();
        // at least one relative link which leads to one absolute
        if (relative->isRelative()) {
            // at least two relative links which lead to one absolute
            relative->reduceRelative();
            // exactly two relative links leading to one absolute
            locationInfo = relative->getRelative();
            // one relative to one absolute
        }
    }

    const CharPosData::Absolute &CharPosData::getLinkedAbsolute() const {
        if (isRelative()) {
            reduceRelative();
            return getRelative()->getAbsolute();
        } else return getAbsolute();
    }

    std::size_t CharPosData::getCharacterIndex() const {
        const Absolute &absolute = getLinkedAbsolute();
        Line *line = *absolute.line;
        CharInfo *info = absolute.info;

        // pointer arithmetic based on characters being sequential
        // returns the number of elements between the first character and this character, which
        // is also the index of this character
        return line == nullptr ? 0 :
               info == nullptr ? line->characters.size() :
               info - &*line->characters.begin();
    }
}