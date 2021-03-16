//
// Created by Computer on 3/16/2021.
//

#ifndef SFML_TEXTBOX_CHARPOS_HPP
#define SFML_TEXTBOX_CHARPOS_HPP

#include <memory>
#include <variant>
#include <cassert>

namespace sftb {
    class Line;
    class CharInfo;

    struct CharPosData {
        friend class TextBox;
        friend class CharPosDataHolder;
    public:
        struct Absolute {
            // nullptr line indicates end of text position
            // any other nullptr info indicates end of line
            Line *line;
            CharInfo *info;
        };
    private:
        using Relative = std::shared_ptr<CharPosData>;

        mutable std::variant<Absolute, Relative> locationInfo;

        [[nodiscard]] Absolute &getAbsolute() const {
            assert(isAbsolute() && "location is not absolute");
            return std::get<Absolute>(locationInfo);
        }

        [[nodiscard]] Relative &getRelative() const {
            assert(isRelative() && "location is not relative");
            return std::get<Relative>(locationInfo);
        }

        [[nodiscard]] bool isAbsolute() const {
            return locationInfo.index() == 0;
        }

        [[nodiscard]] bool isRelative() const {
            return !isAbsolute();
        }

        void reduceRelative() const;

        void updateLine(Line *line) {
            // not getLinkedAbsolute() -- relative is only used to retain references, should never be updated
            getAbsolute().line = line;
        }

        void updateCharInfo(CharInfo *info) {
            getAbsolute().info = info;
        }
    public:
        CharPosData(Line *line, CharInfo *info) : locationInfo(Absolute{line, info}) {}

        CharPosData(const CharPosData &) = delete;
        CharPosData &operator=(const CharPosData &) = delete;
        CharPosData(CharPosData &&) = delete;
        CharPosData &operator=(CharPosData &&) = delete;

        void setRelative(std::shared_ptr<CharPosData> pointer) {
            locationInfo = std::move(pointer);
        }

        const Absolute &getLinkedAbsolute() const;

        [[nodiscard]] std::size_t getCharacterIndex() const;
    };

    /**
     * A relative position within a TextBox, relative to a specific character.
     * If the character referenced by this CharPos is removed, this CharPos will
     * be moved to reference the previous character (which may be the last
     * character on the previous line), or to the character at (0,0) if the CharPos
     * references the first character.
     * A CharPos is used to reference a specific point within the TextBox that
     * stays valid even if the TextBox is edited.
     * Since the CharPos is relative to a TextBox, a CharPos must be only used by
     * the owning TextBox.
     *
     * For example, with the string:
     * HELLO
     * A CharPos could be created for the second character, 'E'. If any of the
     * surrounding characters are removed, or additional characters are added, the
     * CharPos will still reference the same 'E', wherever it is now located.
     * If the 'E' is removed, the CharPos will reference the 'H', instead.
     */
    using CharPos = std::shared_ptr<CharPosData>;
}

#endif //SFML_TEXTBOX_CHARPOS_HPP
