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
#include <list>
#include <cassert>
#include "CharPos.hpp"
#include "InputHandler.hpp"
#include "ScrollBar.hpp"
#include "Reference.hpp"
#include "TextStyle.hpp"
#include "Pos.hpp"
#include "Caret.hpp"
#include "Highlight.hpp"

namespace sf {
    class Font;
    class Event;
}

namespace sftb {
    namespace detail {
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

        class Line;
    }

    class TextBox : public sf::Drawable, public Reference<TextBox> {
        friend class detail::Line;
        friend class Highlight;
    private:
        using Line = detail::Line;
        using CharPosDataHolder = detail::CharPosDataHolder;
        using CharPosData = detail::CharPosData;
        using CharInfo = detail::CharInfo;

        struct LineLengthCompare {
            bool operator()(Line **left, Line **right) const;
        };

        using LineLengthSet = std::multiset<Line **, LineLengthCompare>;

        LineLengthSet lineLength;
        std::vector<Line> lines;
        sf::Font *font;
        std::size_t characterSize;
        float lineHeight, characterWidth;
        sf::Vector2f offset, size;
        mutable std::shared_ptr<bool> redraw;
        ScrollBarManager scrollBarManager;
        sf::Color backgroundColor = sf::Color::Black;
        std::shared_ptr<InputHandler> inputHandler = InputHandler::standard();
        bool selectionActive = false;
        CharPosDataHolder endCharPosDataHolder;
        std::shared_ptr<CaretStyle> caretStyle = std::make_shared<StandardCaretStyle>();
        Caret caret;
        std::list<std::shared_ptr<Highlight>> highlights;

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

        [[nodiscard]] std::size_t getLongestLineLength() const;

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

        ~TextBox() override;

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

        // 0.0 for no rounding
        // 0.5f for rounding at halfway
        // 0.3f for rounding at 0.7f
        // etc.
        [[nodiscard]] Pos getPositionAt(float xOffset, float yOffset, float roundX = 0, float roundY = 0) const {
            xOffset -= getTextOffsetHorizontal();
            yOffset -= getTextOffsetVertical();

            // when non-monospaced fonts are provided this operation may become somewhat more expensive
            // although the != 0 checks are largely pointless now, they may be worthwhile later
            if (roundX != 0) xOffset += roundX * getCharacterWidth();
            if (roundY != 0) yOffset += roundY * getLineHeight();

            return {static_cast<std::size_t>(std::max(0, static_cast<int>(yOffset / getLineHeight()))),
                    static_cast<std::size_t>(std::max(0, static_cast<int>(xOffset / getCharacterWidth())))};
        }

        [[nodiscard]] Pos getPositionAt(const sf::Vector2f &vector, float roundX, float roundY) const {
            return getPositionAt(vector.x, vector.y, roundX, roundY);
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

        void setScrollTo(const Pos &position);

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

        ScrollBarManager &getScrollBarManager() {
            return scrollBarManager;
        }

        const ScrollBarManager &getScrollBarManager() const {
            return scrollBarManager;
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
            return absolute.line == nullptr ? getEndPos() : Pos{getLineIndex(*absolute.line), pos->getCharacterIndex()};
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

        std::shared_ptr<Highlight> highlight(const Pos &first, const Pos &second, std::shared_ptr<Highlighter> highlighter);

        HighlightHandle handledHighlight(const Pos &first, const Pos &second, std::shared_ptr<Highlighter> highlighter) {
            return HighlightHandle(highlight(first, second, std::move(highlighter)));
        }

        void removeHighlight(const std::shared_ptr<Highlight> &highlight);
    };

    namespace detail {
        class CharInfo {
            friend class sftb::TextBox;
            friend class detail::Line;
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

        class Line : public Reference<Line> {
            friend class sftb::TextBox;
            friend class CharPosData;
        private:
            using TextBox = sftb::TextBox;

            TextBox **box;
            TextBox::LineLengthSet::iterator lineLengthIterator;
            std::vector<CharInfo> characters;
            CharPosDataHolder endLineCharPosDataHolder;

            auto createIterator() {
                return getTextBox().lineLength.insert(getReference());
            }

            void removeIterator() {
                getTextBox().lineLength.erase(lineLengthIterator);
            }

            static void prepareRemove(const CharPos &transferPos, const std::vector<CharInfo>::iterator &start,
                                      const std::vector<CharInfo>::iterator &end);
        public:
            explicit Line(TextBox *box) : box(box->getReference()), lineLengthIterator(createIterator()) {
            }

            ~Line() {
                if (box)
                    removeIterator();
            }

            Line(const Line &) = delete;
            Line &operator=(const Line &) = delete;

            Line(Line &&other) noexcept: Reference(std::move(other)), box(other.box), lineLengthIterator(std::move(other.lineLengthIterator)), characters(std::move(other.characters)),
                                         endLineCharPosDataHolder(std::move(other.endLineCharPosDataHolder)) {
                other.box = nullptr;
            }

            Line &operator=(Line &&other) noexcept {
                if(&other != this) {
                    Reference::operator=(std::move(other));
                    if(box)
                    removeIterator();
                    box = other.box;
                    other.box = nullptr;
                    lineLengthIterator = std::move(other.lineLengthIterator);
                    characters = std::move(other.characters);
                    endLineCharPosDataHolder = std::move(other.endLineCharPosDataHolder);
                }
                return *this;
            }

            inline TextBox &getTextBox() {
                assert(box != nullptr && "line is invalid");
                return **box;
            }

            void updateLineLength() {
                removeIterator();
                lineLengthIterator = createIterator();
            }

            [[nodiscard]] std::size_t getNumberCharacters() const {
                assert(box != nullptr && "line is invalid");
                return characters.size();
            }

            void insert(const sf::String &string, std::size_t index = 0);

            void prepareRemoveAll(const CharPos &transferPos) {
                assert(box != nullptr && "line is invalid");
                prepareRemove(transferPos, characters.begin(), characters.end());
                endLineCharPosDataHolder.transfer(transferPos);
            }

            void remove(std::size_t start, std::size_t end = -1);
            void move(Line &line, std::size_t start, std::size_t insertPosition);

            CharInfo &getCharInfo(std::size_t position) {
                assert(position < getNumberCharacters() && "position out of bounds");
                return characters[position];
            }

            [[nodiscard]] const CharInfo &getCharInfo(std::size_t position) const {
                return const_cast<Line *>(this)->getCharInfo(position);
            }
        };
    }
}


#endif //SFML_TEXTBOX_TEXTBOX_HPP
