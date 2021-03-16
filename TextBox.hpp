#ifndef SFML_TEXTBOX_TEXTBOX_HPP
#define SFML_TEXTBOX_TEXTBOX_HPP

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/System/String.hpp>
#include <SFML/System/Clock.hpp>
#include <utility>
#include <variant>
#include <vector>
#include <set>
#include <memory>
#include <cassert>
#include "CharPos.hpp"
#include "InputHandler.hpp"
#include "ScrollBar.hpp"
#include "Reference.hpp"
#include "TextStyle.hpp"
#include "Pos.hpp"
#include "Caret.hpp"

namespace sf {
    class Font;
    class Event;
}

namespace sftb {
    class TextBox : public sf::Drawable, public Reference<TextBox> {
        friend class CharInfo;
        friend class Line;
    private:
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

            void transfer(const CharPos &pos);

            [[nodiscard]] bool active() const {
                return !reference.expired();
            }

            void updateLine(Line &line);
            void updateCharInfo(CharInfo *info);
            CharPos getCharPos(Line *line, CharInfo *info);
        };

        struct LineLengthCompare {
            bool operator()(Line *left, Line *right) const;
        };

        using LineLengthSet = std::multiset<Line *, LineLengthCompare>;

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

        Line &getOrInsertLine(std::size_t line);

        [[nodiscard]] std::size_t getLineIndex(const Line *line) const;

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
                         std::shared_ptr<bool> redraw = nullptr);

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

        [[nodiscard]] sf::Vector2f getContentSize() const;
        [[nodiscard]] Pos getVisibleStart() const;
        [[nodiscard]] Pos getVisibleEnd() const;
        [[nodiscard]] bool isPositionOnScreen(const Pos &position) const;
        [[nodiscard]] Pos getRelativeCharacters(Pos pos, int characters) const;
        [[nodiscard]] Pos getRelativeLine(Pos pos, int lineAmount) const;
        // returns position above pos on screen (for monospaced fonts, this should return the same as getRelativeLine())
        [[nodiscard]] Pos getVisibleRelativeLine(Pos pos, int lineAmount) const;

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

        void setRedrawRequired() const {
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

        [[nodiscard]] std::size_t getLineLength(std::size_t line) const;

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

        void removeLine(unsigned line);
        void removeLines(unsigned start, unsigned end);

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

    class CharInfo {
        friend class TextBox;
        friend class Line;
    private:
        Char c;
        // todo - can this be optimized out? characters are likely to be accessed sequentially, having them more tightly packed
        //  could improve performance (in addition to being more memory friendly!)
        TextBox::CharPosDataHolder referenceHolder;
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

    class Line : public Reference<Line> {
    public:
        TextBox::LineLengthSet::iterator lineLengthIterator;
        std::vector<CharInfo> characters;
        TextBox::CharPosDataHolder endLineCharPosDataHolder;

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

        void insert(TextBox *box, const sf::String &string, std::size_t index = 0);

        static void prepareRemove(const CharPos &transferPos, const std::vector<CharInfo>::iterator &start,
                                  const std::vector<CharInfo>::iterator &end);

        void prepareRemoveAll(const CharPos &transferPos) {
            prepareRemove(transferPos, characters.begin(), characters.end());
            endLineCharPosDataHolder.transfer(transferPos);
        }

        void remove(TextBox *box, std::size_t start, std::size_t end = -1);
        void move(TextBox *box, Line &line, std::size_t start, std::size_t insertPosition);

        CharInfo &getCharInfo(std::size_t position) {
            assert(position < getNumberCharacters() && "position out of bounds");
            return characters[position];
        }

        [[nodiscard]] const CharInfo &getCharInfo(std::size_t position) const {
            return const_cast<Line *>(this)->getCharInfo(position);
        }
    };
}


#endif //SFML_TEXTBOX_TEXTBOX_HPP
