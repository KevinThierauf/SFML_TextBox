#ifndef SFML_TEXTBOX_TEXTBOX_HPP
#define SFML_TEXTBOX_TEXTBOX_HPP

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/System/String.hpp>
#include <utility>
#include <vector>
#include <set>
#include <memory>
#include <cassert>
#include <iosfwd>
#include "ScrollBar.hpp"
#include "Reference.hpp"

namespace sf {
    class Font;
    class Event;
}

namespace sftb {
    class TextBox;

    using Char = sf::Uint32;

    class InputHandler {
        friend class TextBox;
    private:
        TextBox *textBox = nullptr;
    protected:
        static bool isNumeric(Char code) {
            return '0' <= code && code <= '9';
        }

        static bool isAlpha(Char code) {
            return ('A' <= code && code <= 'Z') || ('a' <= code && code <= 'z');
        }

        static bool isWhitespace(Char code) {
            return code == ' ' || code == '\t' || code == '\n' || code == '\r';
        }

        static bool isSymbol(Char code) {
            return ('!' <= code && code <= '/') || (':' <= code && code <= '@') || ('[' <= code && code <= '`') ||
                   ('{' <= code && code <= '~');
        }

    public:
        InputHandler() = default;

        InputHandler(const InputHandler &) : textBox(nullptr) {}

        InputHandler &operator=(const InputHandler &other) {
            if (this != &other)
                textBox = nullptr;
            return *this;
        }

        InputHandler(InputHandler &&) = delete;
        InputHandler &operator=(InputHandler &&) = delete;

        static std::unique_ptr<InputHandler> basic() {
            return std::make_unique<InputHandler>();
        }

        static std::unique_ptr<InputHandler> standard();

        virtual ~InputHandler() = default;

        [[nodiscard]] TextBox &getTextBox() {
            return *textBox;
        }

        [[nodiscard]] const TextBox &getTextBox() const {
            return *textBox;
        }

        [[nodiscard]] virtual bool isTextInput(Char code) const {
            return isAlpha(code) || isNumeric(code) || isWhitespace(code) || isSymbol(code);
        }

        virtual void handle(sf::Keyboard::Key key, bool pressed, bool control, bool shift, bool alt) {
        }
    };

    class TextBox : public sf::Drawable, public Reference<TextBox> {
    private:
        struct Line;
        struct CharInfo;
        struct CharPosData;
    public:

        /**
         * An absolute position within a TextBox.
         */
        struct Pos {
            std::size_t line, position;
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

        /**
         * Text styling. Each character is styled according to one TextStyle object; styles
         * do not overlap, but may span several consecutive characters or lines.
         *
         * TextStyles are created and managed by their TextBox.
         */
        struct TextStyle {
            friend class TextBox;
        private:
            std::shared_ptr<bool> redraw;
            sf::Color textColor;
            bool bold, italic;
            bool underline, strikethrough;

            TextStyle(std::shared_ptr<bool> redraw, const sf::Color &textColor, bool bold, bool italic, bool underline,
                      bool strikethrough) : redraw(std::move(redraw)), textColor(textColor), bold(bold), italic(italic),
                                            underline(underline), strikethrough(strikethrough) {}

        public:
            TextStyle &pasteFrom(const TextStyle &style) {
                textColor = style.textColor;
                bold = style.bold;
                italic = style.italic;
                underline = style.underline;
                strikethrough = style.strikethrough;
                *redraw = true;

                return *this;
            }

            [[nodiscard]] const sf::Color &getTextColor() const {
                return textColor;
            }

            TextStyle &setTextColor(const sf::Color &color) {
                textColor = color;
                *redraw = true;
                return *this;
            }

            TextStyle &setBold(bool b) {
                bold = b;
                *redraw = true;
                return *this;
            }

            [[nodiscard]] bool isBold() const {
                return bold;
            }

            TextStyle &setItalic(bool i) {
                italic = i;
                *redraw = true;
                return *this;
            }

            [[nodiscard]] bool isItalic() const {
                return italic;
            }

            TextStyle &setUnderline(bool u) {
                underline = u;
                *redraw = true;
                return *this;
            }

            [[nodiscard]] bool isUnderline() const {
                return underline;
            }

            TextStyle &setStrikethrough(bool s) {
                strikethrough = s;
                *redraw = true;
                return *this;
            }

            [[nodiscard]] bool isStrikethrough() const {
                return strikethrough;
            }
        };

        // todo
        class StyledRange {
        private:
            TextBox *reference;
            CharPos pos;
        public:
            TextBox &getTextBox() {
                return *reference;
            }

            [[nodiscard]] Pos getStartPosition() const {
                return reference->getPositionOfChar(pos);
            }

            [[nodiscard]] Pos getEndPosition() const {
                // todo
            }

            StyledRange *getNextStyle() {
                // todo
            }

            StyledRange *getPreviousStyle() {
                // todo
            }
        };

        // todo - multiple caret support
        class Caret {
            friend class TextBox;
        private:
            TextBox *reference;
            CharPos pos, selectionEndPos;

            Caret(TextBox &box, Pos position = {}) : reference(box.getReference()),
                                                     pos(reference->getCharPos(position)), selectionEndPos(nullptr) {
            }

            Caret(Caret &&) = default;
            Caret &operator=(Caret &&) = default;

            [[nodiscard]] Pos getClosestPos(const Pos &position) const {
                auto line = std::min(reference->getNumberLines(), position.line);
                return {line, std::min(reference->getLineLength(line), position.position)};
            }

        public:
            Caret(const Caret &) = delete;
            Caret &operator=(const Caret &) = delete;

            TextBox &getTextBox() {
                return *reference;
            }

            [[nodiscard]] Pos getPosition() const {
                return reference->getPositionOfChar(pos);
            }

            void setPosition(const Pos &position) {
                pos = reference->getCharPos(position);
                selectionEndPos = nullptr;
            }

            void setClosestPosition(const Pos &position) {
                setPosition(getClosestPos(position));
            }

            [[nodiscard]] bool hasSelection() const {
                return selectionEndPos != nullptr;
            }

            [[nodiscard]] Pos getSelectionEndPos() const {
                return reference->getPositionOfChar(selectionEndPos);
            }

            void setSelectionEndPos(const Pos &position) {
                selectionEndPos = reference->getCharPos(position);
            }

            void setSelectionEndClosestPosition(const Pos &position) {
                setSelectionEndPos(getClosestPos(position));
            }

            void removeSelection() {
                selectionEndPos = nullptr;
            }

            void removeSelectedText() {
                if (hasSelection())
                    reference->removeText(getPosition(), getSelectionEndPos());
            }

            [[nodiscard]] sf::String getSelectedText() const {
                return hasSelection() ? reference->getTextFrom(getPosition(), getSelectionEndPos()) : "";
            }

            void insert(const sf::String &string) {
                removeSelection();
                setPosition(reference->insertText(getPosition(), string));
            }

            void move(int line, int position) {
                // todo - for monospaced fonts should use position of character on screen when moving
                //  caret line, so that the caret is moved to the character above on the screen (instead
                //  of just the character on the previous line with the same position).
            }

            void moveSelectionEnd(int line, int position) {
                // todo
            }
        };
    private:
        struct CharPosData {
            // nullptr line indicates end of text position
            // any other nullptr info indicates end of line
            Line *line;
            CharInfo *info;

            CharPosData(Line *line, CharInfo *info) : line(line), info(info) {}

            CharPosData(const CharPosData &) = delete;
            CharPosData &operator=(const CharPosData &) = delete;
            CharPosData(CharPosData &&) = delete;
            CharPosData &operator=(CharPosData &&) = delete;

            [[nodiscard]] std::size_t getCharacterIndex() const {
                // pointer arithmetic based on characters being sequential
                // returns the number of elements between the first character and this character, which
                // is also the index of this character
                return line == nullptr ? 0 :
                       info == nullptr ? line->characters.size() :
                       info - &*line->characters.begin();
            }
        };

        class CharPosDataHolder {
        private:
            std::weak_ptr<CharPosData> reference;

        public:
            CharPosDataHolder() = default;
            CharPosDataHolder(const CharPosDataHolder &) = delete;
            CharPosDataHolder &operator=(const CharPosDataHolder &) = delete;
            CharPosDataHolder(CharPosDataHolder &&) = default;
            CharPosDataHolder &operator=(CharPosDataHolder &&) = default;

            ~CharPosDataHolder() {
                if (active()) {
                    auto pointer = reference.lock();
                    // todo - move references to appropriate character
                }
            }

            [[nodiscard]] bool active() const {
                return !reference.expired();
            }

            void updateLine(Line &line) {
                if (active()) reference.lock()->line = line.getReference();
            }

            void updateCharInfo(CharInfo *info) {
                if (active()) reference.lock()->info = info;
            }

            CharPos getCharPos(Line *line, CharInfo *info) {
                if (active()) return reference.lock();
                CharPos charPos = std::make_shared<CharPosData>(line, info);
                reference = charPos;
                return charPos;
            }
        };

        class CharInfo {
            friend class TextBox;
        private:
            Char c;
            // todo - can this be optimized out? characters are likely to be accessed sequentially, having them more tightly packed
            //  could improve performance (in addition to being more memory friendly!)
            CharPosDataHolder referenceHolder;
        public:
            explicit CharInfo(Char c) : c(c) {}

            // while copying could be implemented, there is currently no intended
            // reason to copy CharInfo. Operators are deleted for safety.
            CharInfo(const CharInfo &) = delete;
            CharInfo &operator=(const CharInfo &) = delete;

            CharInfo(CharInfo &&other) noexcept: c(other.c), referenceHolder(std::move(other.referenceHolder)) {
                referenceHolder.updateCharInfo(this);
            }

            CharInfo &operator=(CharInfo &&other) noexcept {
                c = other.c;
                referenceHolder = std::move(other.referenceHolder);
                referenceHolder.updateCharInfo(this);

                return *this;
            }

            [[nodiscard]] Char getChar() const {
                return c;
            }
        };

        struct LineLengthCompare {
            bool operator()(Line *left, Line *right) const;
        };

        using LineLengthSet = std::multiset<Line *, LineLengthCompare>;

        struct Line : Reference<Line> {
            LineLengthSet::iterator lineLengthIterator;
            std::vector<CharInfo> characters;
            CharPosDataHolder endLineCharPosDataHolder;

            auto createIterator(TextBox *box) {
                return box->lineLength.insert(getReference());
            }

            explicit Line(TextBox *box) : lineLengthIterator(createIterator(box)) {
            }

            Line(const Line &) = delete;
            Line &operator=(const Line &) = delete;
            Line(Line &&) = default;
            Line &operator=(Line &&) = default;

            void updateLineLength(TextBox *box) {
                box->lineLength.erase(lineLengthIterator);
                lineLengthIterator = createIterator(box);
            }

            [[nodiscard]] std::size_t getNumberCharacters() const {
                return characters.size();
            }

            void insert(TextBox *box, const sf::String &string, std::size_t index = 0) {
                assert(index <= getNumberCharacters() && "index out of bounds");
                for (char c : string) {
                    // todo - optimize (will push characters back multiple times)
                    characters.emplace(characters.begin() + index++, c);
                }
                updateLineLength(box);
            }

            void remove(TextBox *box, std::size_t start, std::size_t end = -1) {
                characters.erase(characters.begin() + start, characters.begin() + std::min(end, characters.size()));
                updateLineLength(box);
            }

            void move(TextBox *box, Line &line, std::size_t start, std::size_t insertPosition) {
                assert(start <= getNumberCharacters() && "start out of bounds");
                assert(insertPosition <= line.getNumberCharacters() && "insert position out of bounds");
                // todo - clean up

                // move characters (at and after start) to other line at insertPosition
                auto iterFirstCharacter = characters.begin() + start;
                auto iterLastCharacter = characters.end();

                // update character line
                auto iter = iterFirstCharacter;
                while (iter < characters.end()) {
                    iter->referenceHolder.updateLine(line);
                    iter++;
                }

                line.characters.insert(line.characters.begin() + insertPosition,
                                       std::make_move_iterator(iterFirstCharacter),
                                       std::make_move_iterator(iterLastCharacter));
                characters.erase(iterFirstCharacter, iterLastCharacter);
                line.updateLineLength(box);
                updateLineLength(box);
            }

            CharInfo &getCharInfo(std::size_t position) {
                assert(position < getNumberCharacters() && "position out of bounds");
                return characters[position];
            }

            [[nodiscard]] const CharInfo &getCharInfo(std::size_t position) const {
                return const_cast<Line *>(this)->getCharInfo(position);
            }
        };

        std::vector<Line> lines;
        LineLengthSet lineLength;
        sf::Font *font;
        std::size_t characterSize;
        float lineHeight, characterWidth;
        sf::Vector2f offset, size;
        mutable std::shared_ptr<bool> redraw;
        ScrollBarManager scrollBarManager;
        sf::Color backgroundColor = sf::Color::Black;
        std::unique_ptr<InputHandler> inputHandler = InputHandler::standard();
        bool selectionActive = false;
        CharPosDataHolder endCharPosDataHolder;
        Caret caret;

        Line &getLine(std::size_t line) {
            assert(line < getNumberLines() && "line out of bounds");
            return lines[line];
        }

        const Line &getLine(std::size_t line) const {
            return const_cast<TextBox *>(this)->getLine(line);
        }

        Line &getOrInsertLine(std::size_t line) {
            assert(line <= getNumberLines() && "line out of bounds");
            return getNumberLines() == line ? *lines.emplace(lines.begin() + line, this) : lines[line];
        }

        [[nodiscard]] std::size_t getLineIndex(const Line *line) const {
            assert(line != nullptr && "line is nullptr");
            // same logic as CharInfo getCharacterIndex
            return line - &*lines.begin();
        }

        [[nodiscard]] float getCharacterWidth() const {
            // todo support non monospaced fonts
            return characterWidth;
        }

        [[nodiscard]] Line &getLongestLine() const {
            return **lineLength.begin();
        }

        // return true if verify is true, and either x or y are outside this TextBox
        bool isOutBounds(bool verify, int x, int y) const;
        static CharPos getCharPos(std::weak_ptr<CharPosData> &reference, Line *line, CharInfo *info);
    protected:
        void draw(sf::RenderTarget &target, sf::RenderStates states) const override;
    public:
        explicit TextBox(sf::Font &font, sf::Vector2f size, std::size_t characterSize = 16,
                         std::shared_ptr<bool> redraw = nullptr)
                : font(&font), size(size), characterSize(characterSize),
                  lineHeight(font.getLineSpacing(characterSize)),
                  characterWidth(font.getGlyph('a', characterSize, false).advance),
                  redraw(redraw ? std::move(redraw) : std::make_shared<bool>(true)),
                  scrollBarManager(this->redraw, [box(getReference())]() {
                      return box->getContentSize();
                  }, [box(getReference())] {
                      return box->getSize();
                  }), caret(*this) {
            inputHandler->textBox = getReference();
        }

        // it may make sense to allow explicit copy operations, but unnecessary for now
        TextBox(const TextBox &) = delete;
        TextBox &operator=(const TextBox &) = delete;

        TextBox(TextBox &&other) = default;
        TextBox &operator=(TextBox &&) = default;

        [[nodiscard]] float getVerticalOffset() const {
            return offset.y + scrollBarManager.getVerticalScrollBar().getScrollOffset();
        }

        [[nodiscard]] float getHorizontalOffset() const {
            return offset.x + scrollBarManager.getHorizontalScrollBar().getScrollOffset();
        }

        [[nodiscard]] const sf::Vector2f &getSize() const {
            return size;
        }

        void setSize(const sf::Vector2f &s) {
            size = s;
            setRedrawRequired();
        }

        [[nodiscard]] float getLineHeight() const {
            return lineHeight;
        }

        [[nodiscard]] Pos getPositionAt(float xOffset, float yOffset) const {
            xOffset -= getHorizontalOffset();
            yOffset -= getVerticalOffset();

            return {static_cast<std::size_t>(std::max(0, static_cast<int>(yOffset / getLineHeight()))),
                    static_cast<std::size_t>(std::max(0, static_cast<int>(xOffset / getCharacterWidth())))};
        }

        [[nodiscard]] Pos getPositionAt(const sf::Vector2f &vector) const {
            return getPositionAt(vector.x, vector.y);
        }

        [[nodiscard]] sf::Vector2f getOffsetOf(const Pos &pos) const {
            return {
                    getHorizontalOffset() + static_cast<float>(pos.position) * getCharacterWidth(),
                    getVerticalOffset() + static_cast<float>(pos.line) * getLineHeight()
            };
        }

        [[nodiscard]] sf::Vector2f getContentSize() const {
            return offset + sf::Vector2f{
                    getLongestLine().getNumberCharacters() * getCharacterWidth(),
                    getNumberLines() * getLineHeight()
            };
        }

        [[nodiscard]] Pos getRelative(Pos pos, int characters) const {
            if (characters < 0) {
                characters = -characters;
                do {
                    if (pos.position >= characters) {
                        pos.position -= characters;
                        return pos;
                    }

                    if(pos.line == 0) {
                        pos.position = 0;
                        return pos;
                    }

                    characters -= static_cast<int>(pos.position) + 1;
                    pos.position = getLineLength(--pos.line);
                } while (characters > 0);
            } else {
                do {
                    std::size_t lengthLine = getLineLength(pos.line);
                    if (lengthLine - pos.position >= characters) {
                        pos.position += characters;
                        return pos;
                    }

                    if(pos.line == getNumberLines()) {
                        pos.position = 0;
                        return pos;
                    }

                    characters -= static_cast<int>(lengthLine - pos.position) + 1;
                    pos.position = 0;
                    pos.line++;
                } while (characters > 0);
            }

            return pos;
        }

        [[nodiscard]] sf::Font &getFont() const {
            return *font;
        }

        void setFont(sf::Font &f) {
            font = &f;
            setRedrawRequired();
        }

        [[nodiscard]] std::size_t getCharacterSize() const {
            return characterSize;
        }

        void setCharacterSize(std::size_t s) {
            characterSize = s;
            setRedrawRequired();
        }

        [[nodiscard]] bool isRedrawRequired() {
            return *redraw;
        }

        void setRedrawRequired() {
            *redraw = true;
        }

        void setRedrawReference(std::shared_ptr<bool> r) {
            assert(r != nullptr && "redraw reference is nullptr");
            if (*redraw) *r = true; // retain redraw status
            redraw = std::move(r);
        }

        [[nodiscard]] std::shared_ptr<bool> getRedrawReference() {
            return redraw;
        }

        const sf::Color &getBackgroundColor() const {
            return backgroundColor;
        }

        void setBackgroundColor(const sf::Color &color) {
            backgroundColor = color;
            setRedrawRequired();
        }

        InputHandler &getInputHandler() {
            return *inputHandler;
        }

        void setInputHandler(std::unique_ptr<InputHandler> input) {
            assert(input && "input is nullptr");
            inputHandler = std::move(input);
            inputHandler->textBox = getReference();
        }

        [[nodiscard]] std::size_t getNumberLines() const {
            return lines.size();
        }

        [[nodiscard]] std::size_t getLineLength(std::size_t line) const {
            return line == getNumberLines() ? 0 : getLine(line).getNumberCharacters();
        }

        [[nodiscard]] Pos getStartPos() const {
            // included to make code more readable
            return {0, 0};
        }

        [[nodiscard]] Pos getEndPos() const {
            return {getNumberLines(), 0};
        }

        CharPos getCharPos(const Pos &pos);

        [[nodiscard]] Pos getPositionOfChar(const CharPos &pos) const {
            assert(pos && "empty CharPos");
            return pos->line == nullptr ? getEndPos() : Pos{getLineIndex(pos->line), pos->getCharacterIndex()};
        }

        [[nodiscard]] sf::String getTextFrom(Pos first, Pos second) const;
        [[nodiscard]] sf::String getLineContents(std::size_t line, std::size_t start = 0, std::size_t end = -1) const;

        Pos insertText(Pos pos, const sf::String &text);
        Pos insertLine(unsigned line, const sf::String &string = "");
        void removeText(Pos from, Pos to);

        Pos replaceText(const Pos &from, const Pos &to, const sf::String &text) {
            removeText(from, to);
            return insertText(from, text);
        }

        void removeLine(unsigned line) {
            assert(line < getNumberLines() && "line out of bounds");
            lines.erase(lines.begin() + line);
        }

        void removeLines(unsigned start, unsigned end) {
            assert(start <= getNumberLines() &&
                   "start out of bounds"); // could omit check (implied by checks below) but may help debugging
            assert(end <= getNumberLines() && "end out of bounds");
            assert(start <= end && "start must be before end");
            lines.erase(lines.begin() + start, lines.begin() + end);
        }

        void handleEvent(const sf::Event &event, bool verifyArea = true);
        void handleInput(sf::Keyboard::Key key, bool pressed, bool control, bool shift, bool alt);
        void handleTextInput(const sf::String &string);
        void handleScroll(bool vertical, float amount);
        void handleInput(sf::Mouse::Button button, bool pressed, int x, int y);
        void handleMousePositionChange(int x, int y);

        [[nodiscard]] Caret &getPrimaryCaret() {
            return caret;
        }

        [[nodiscard]] const Caret &getPrimaryCaret() const {
            return caret;
        }
    };

    inline bool operator<(const TextBox::Pos &first, const TextBox::Pos &second) {
        return first.line == second.line ? first.position < second.position : first.line < second.line;
    }

    inline bool operator<=(const TextBox::Pos &first, const TextBox::Pos &second) {
        return !(second < first);
    }

    inline bool operator>(const TextBox::Pos &first, const TextBox::Pos &second) {
        return second < first;
    }

    inline bool operator>=(const TextBox::Pos &first, const TextBox::Pos &second) {
        return !(first < second);
    }

    inline bool operator==(const TextBox::Pos &first, const TextBox::Pos &second) {
        return first.line == second.line && first.position == second.position;
    }

    inline bool operator!=(const TextBox::Pos &first, const TextBox::Pos &second) {
        return !(first == second);
    }

    inline TextBox::Pos operator+(const TextBox::Pos &first, const TextBox::Pos &second) {
        return {first.line + second.line, first.position + second.position};
    }

    std::ostream &operator<<(std::ostream &out, const TextBox::Pos &position);
}


#endif //SFML_TEXTBOX_TEXTBOX_HPP
