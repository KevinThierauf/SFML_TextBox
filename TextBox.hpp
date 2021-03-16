#ifndef SFML_TEXTBOX_TEXTBOX_HPP
#define SFML_TEXTBOX_TEXTBOX_HPP

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/System/String.hpp>
#include <SFML/System/Clock.hpp>
#include <utility>
#include <variant>
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

        class Caret;

        class CaretStyle {
            friend class Caret;
        protected:
            virtual void draw(sf::RenderTarget &target, sf::RenderStates states, const Caret &caret) = 0;

            virtual void notifyPositionChange(const Caret &c, const Pos &previousPosition) {
            }

        public:
            CaretStyle() = default;
            CaretStyle(CaretStyle &) = delete;
            CaretStyle &operator=(CaretStyle &) = delete;
            CaretStyle(CaretStyle &&) = delete;
            CaretStyle &operator=(CaretStyle &&) = delete;

            virtual ~CaretStyle() = default;
        };

        class StandardCaretStyle : public CaretStyle {
        public:
            static constexpr float DEFAULT_CARET_WIDTH = 2;
            static constexpr int DEFAULT_CARET_BLINK_WAIT = 2000;
            static constexpr int DEFAULT_CARET_BLINK_PERIOD = 2000;
        private:
            sf::Color firstColor;
            sf::Color secondColor;
            sf::Clock clock;

            float caretWidth = DEFAULT_CARET_WIDTH;
            int caretBlinkWait = DEFAULT_CARET_BLINK_WAIT;
            int caretBlinkPeriod = DEFAULT_CARET_BLINK_PERIOD;

            sf::Int32 lastPositionChange = 0;
        protected:
            void draw(sf::RenderTarget &target, sf::RenderStates states, const Caret &caret) override;
            void notifyPositionChange(const Caret &caret, const Pos &previousPosition) override;
        public:
            explicit StandardCaretStyle(sf::Color firstColor = sf::Color::White,
                                        sf::Color secondColor = sf::Color::Transparent)
                    : firstColor(firstColor), secondColor(secondColor) {}

            float getBlinkPercent();
            sf::Color getCurrentCaretColor();
            sf::Vector2f getCaretPosition(const Caret &caret);
            sf::Vector2f getCaretSize(const Caret &caret);

            [[nodiscard]] const sf::Color &getFirstColor() const {
                return firstColor;
            }

            void setFirstColor(const sf::Color &color) {
                firstColor = color;
            }

            [[nodiscard]] const sf::Color &getSecondColor() const {
                return secondColor;
            }

            void setSecondColor(const sf::Color &color) {
                secondColor = color;
            }
        };

        // todo - multiple caret support
        class Caret : public sf::Drawable {
            friend class TextBox;
        private:
            TextBox *reference;
            CharPos pos, selectionEndPos;
            std::shared_ptr<CaretStyle> style;

            explicit Caret(TextBox &box, Pos position = {}) :
                    reference(box.getReference()), pos(reference->getCharPos(position)), selectionEndPos(nullptr),
                    style(reference->getCaretStyle()) {
            }

            Caret(Caret &&) = default;
            Caret &operator=(Caret &&) = default;

            [[nodiscard]] Pos getClosestPos(const Pos &position) const {
                auto line = std::min(reference->getNumberLines(), position.line);
                return {line, std::min(reference->getLineLength(line), position.position)};
            }

        protected:
            void draw(sf::RenderTarget &target, sf::RenderStates states) const override {
                style->draw(target, states, *this);
            }

        public:
            Caret(const Caret &) = delete;
            Caret &operator=(const Caret &) = delete;

            TextBox &getTextBox() {
                return *reference;
            }

            [[nodiscard]] std::shared_ptr<CaretStyle> getCaretStyle() const {
                return style;
            }

            void setCaretStyle(std::shared_ptr<CaretStyle> s) {
                assert(s != nullptr && "s is nullptr");
                style = std::move(s);
            }

            [[nodiscard]] Pos getPosition() const {
                return reference->getPositionOfChar(pos);
            }

            void setPosition(const Pos &position);

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
                removeSelectedText();
                setPosition(reference->insertText(getPosition(), string));
            }
        };
    private:
        struct CharPosData {
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

            Absolute &getAbsolute() const {
                assert(isAbsolute() && "location is not absolute");
                return std::get<Absolute>(locationInfo);
            }

            Relative &getRelative() const {
                assert(isRelative() && "location is not relative");
                return std::get<Relative>(locationInfo);
            }

            bool isAbsolute() const {
                return locationInfo.index() == 0;
            }

            bool isRelative() const {
                return !isAbsolute();
            }

            void reduceRelative() const {
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

        public:
            CharPosData(Line *line, CharInfo *info) : locationInfo(Absolute{line, info}) {}

            CharPosData(const CharPosData &) = delete;
            CharPosData &operator=(const CharPosData &) = delete;
            CharPosData(CharPosData &&) = delete;
            CharPosData &operator=(CharPosData &&) = delete;

            void setRelative(std::shared_ptr<CharPosData> pointer) {
                locationInfo = std::move(pointer);
            }

            const Absolute &getLinkedAbsolute() const {
                if (isRelative()) {
                    reduceRelative();
                    return getRelative()->getAbsolute();
                } else return getAbsolute();
            }

            [[nodiscard]] std::size_t getCharacterIndex() const {
                const Absolute &absolute = getLinkedAbsolute();
                Line *line = absolute.line;
                CharInfo *info = absolute.info;

                // pointer arithmetic based on characters being sequential
                // returns the number of elements between the first character and this character, which
                // is also the index of this character
                return line == nullptr ? 0 :
                       info == nullptr ? line->characters.size() :
                       info - &*line->characters.begin();
            }

            void updateLine(Line *line) {
                // not getLinkedAbsolute() -- relative is only used to retain references, should never be updated
                getAbsolute().line = line;
            }

            void updateCharInfo(CharInfo *info) {
                getAbsolute().info = info;
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
                assert(!active() && "CharPosDataHolder info was not transferred");
            }

            void transfer(const CharPos &pos) {
                if (active()) {
                    reference.lock()->setRelative(pos);
                    reference.reset();
                }
            }

            [[nodiscard]] bool active() const {
                return !reference.expired();
            }

            void updateLine(Line &line) {
                if (active()) reference.lock()->updateLine(line.getReference());
            }

            void updateCharInfo(CharInfo *info) {
                if (active()) reference.lock()->updateCharInfo(info);
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

            static void prepareRemove(const CharPos &transferPos, const std::vector<CharInfo>::iterator &start,
                                      const std::vector<CharInfo>::iterator &end) {
                for (auto iter = start; iter < end; iter++) {
                    iter->referenceHolder.transfer(transferPos);
                }
            }

            void prepareRemoveAll(const CharPos &transferPos) {
                prepareRemove(transferPos, characters.begin(), characters.end());
                endLineCharPosDataHolder.transfer(transferPos);
            }

            void remove(TextBox *box, std::size_t start, std::size_t end = -1) {
                auto endIndex = std::min(end, characters.size());

                CharPos transferPos;

                if (start == 0) {
                    auto lineIndex = box->getLineIndex(this);
                    if (lineIndex == 0) {
                        // end character if exists, or end of line
                        transferPos = box->getCharPos({lineIndex, endIndex});
                    } else {
                        Line &previousLine = box->getLine(lineIndex - 1);
                        transferPos = previousLine.endLineCharPosDataHolder.getCharPos(&previousLine, nullptr);
                    }
                } else {
                    CharInfo &info = characters[start - 1];
                    transferPos = info.referenceHolder.getCharPos(this, &info);
                }

                auto iterStart = characters.begin() + start;
                auto iterEnd = characters.begin() + endIndex;
                prepareRemove(transferPos, iterStart, iterEnd);

                characters.erase(iterStart, iterEnd);
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
        std::shared_ptr<CaretStyle> caretStyle = std::make_shared<StandardCaretStyle>();
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

        CharPos getTransferPos(std::size_t start, std::size_t end) {
            return start == 0 ? getCharPos({end, getLineLength(end)}) :
                   getCharPos({start - 1, getLineLength(start - 1)});
        }

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

        [[nodiscard]] float getTextOffsetVertical() const {
            return offset.y + scrollBarManager.getVerticalScrollBar().getScrollOffset();
        }

        [[nodiscard]] float getTextOffsetHorizontal() const {
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
            xOffset -= getTextOffsetHorizontal();
            yOffset -= getTextOffsetVertical();

            return {static_cast<std::size_t>(std::max(0, static_cast<int>(yOffset / getLineHeight()))),
                    static_cast<std::size_t>(std::max(0, static_cast<int>(xOffset / getCharacterWidth())))};
        }

        [[nodiscard]] Pos getPositionAt(const sf::Vector2f &vector) const {
            return getPositionAt(vector.x, vector.y);
        }

        [[nodiscard]] sf::Vector2f getOffsetOf(const Pos &pos) const {
            return {
                    getTextOffsetHorizontal() + static_cast<float>(pos.position) * getCharacterWidth(),
                    getTextOffsetVertical() + static_cast<float>(pos.line) * getLineHeight()
            };
        }

        [[nodiscard]] sf::Vector2f getContentSize() const {
            return offset + sf::Vector2f{
                    getLongestLine().getNumberCharacters() * getCharacterWidth(),
                    getNumberLines() * getLineHeight()
            };
        }

        [[nodiscard]] Pos getVisibleStart() const;
        [[nodiscard]] Pos getVisibleEnd() const;
        [[nodiscard]] bool isPositionOnScreen(const Pos &position) const;

        [[nodiscard]] Pos getRelativeCharacters(Pos pos, int characters) const;

        [[nodiscard]] Pos getRelativeLine(Pos pos, int lineAmount) const {
            if (lineAmount > 0) {
                pos.line += lineAmount;
                if (pos.line >= getNumberLines()) return getEndPos();
            } else {
                lineAmount = -lineAmount;
                if (lineAmount == pos.line)
                    pos.line = 0;
                else if (lineAmount > pos.line) return getStartPos();
                else pos.line -= lineAmount;
            }
            pos.position = std::min(pos.position, getLineLength(pos.line));
            return pos;
        }

        // returns position above pos on screen (for monospaced fonts, this should return the same as getRelativeLine())
        [[nodiscard]] Pos getVisibleRelativeLine(Pos pos, int lineAmount) const {
            Pos relativePosition = getRelativeLine(pos, lineAmount);
            // if getRelativeLine needs to modify the position, use that value instead
            if (relativePosition.position != pos.position) return relativePosition;

            auto line = relativePosition.line;
            float xPos = getOffsetOf(pos).x;
            auto position = getPositionAt({xPos, line * getLineHeight()}).position;
            return {line, std::min(position, getLineLength(line))};
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

        std::shared_ptr<CaretStyle> getCaretStyle() const {
            return caretStyle;
        }

        void setCaretStyle(std::shared_ptr<CaretStyle> style, bool applyToExisting = true) {
            assert(style != nullptr && "style is nullptr");
            caretStyle = std::move(style);
            if (applyToExisting) {
                getPrimaryCaret().setCaretStyle(caretStyle);
            }
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
            const CharPosData::Absolute &absolute = pos->getLinkedAbsolute();
            return absolute.line == nullptr ? getEndPos() : Pos{getLineIndex(absolute.line), pos->getCharacterIndex()};
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
            getLine(line).prepareRemoveAll(getTransferPos(line, line + 1));
            lines.erase(lines.begin() + line);
        }

        void removeLines(unsigned start, unsigned end) {
            assert(start <= getNumberLines() &&
                   "start out of bounds"); // could omit check (implied by checks below) but may help debugging
            assert(end <= getNumberLines() && "end out of bounds");
            assert(start <= end && "start must be before end");
            // todo - try and optimize -- iterating over each removed character from each removed line in case
            //  transfer is required is fairly inefficient
            CharPos transfer = getTransferPos(start, end);
            auto iterStart = lines.begin() + start;
            auto iterEnd = lines.begin() + end;
            for (auto iter = iterStart; iter < iterEnd; iter++) {
                iter->prepareRemoveAll(transfer);
            }
            lines.erase(iterStart, iterEnd);
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
