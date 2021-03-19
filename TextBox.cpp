#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Window/Event.hpp>
#include <cmath>
#include "TextBox.hpp"
#include "ScrollBarStyle.hpp"

namespace sftb {
    // used to determine by how much to round when selecting character
    // 0.4 means that, when clicking on a character, there is a 20% preference to the leftmost character
    // 0.5 for no preference
    constexpr float CHARACTER_ROUNDING = 0.4f;

    TextBox::TextBox(sf::Font &font, sf::Vector2f size, std::size_t characterSize, std::shared_ptr<bool> redraw)
            : font(&font), size(size), characterSize(characterSize),
              lineHeight(font.getLineSpacing(characterSize)),
              characterWidth(font.getGlyph('a', characterSize, false).advance),
              redraw(redraw ? std::move(redraw) : std::make_shared<bool>(true)),
              scrollBarManager(this->redraw, [box(getReference())]() {
                  return (**box).getContentSize();
              }, [box(getReference())] {
                  return (**box).getSize();
              }), caret(*this) {
        inputHandler->textBox = getReference();
    }

    TextBox::~TextBox() {
        for (auto &highlight : highlights) {
            highlight->box = nullptr;
        }
    }

    bool TextBox::LineLengthCompare::operator()(Line **left, Line **right) const {
        return (**right).getNumberCharacters() < (**left).getNumberCharacters();
    }

    void TextBox::removeHighlight(const std::shared_ptr<Highlight> &highlight) {
        assert(highlight != nullptr && "highlight is nullptr");
        highlight->box = nullptr;
        highlights.erase(highlight->iterator);
    }

    std::shared_ptr<Highlight> TextBox::highlight(const Pos &first, const Pos &second, std::shared_ptr<Highlighter> highlighter) {
        auto iter = highlights.emplace(highlights.begin(), std::make_shared<Highlight>(Highlight(*this, std::move(highlighter), first, second)));
        (**iter).iterator = iter;
        return *iter;
    }

    void TextBox::draw(sf::RenderTarget &target, sf::RenderStates states) const {
        *redraw = false;

        // draw background
        sf::RectangleShape background(getSize());
        background.setFillColor(backgroundColor);
        target.draw(background);

        // draw text
        Pos startPos = getVisibleStart();
        Pos endPos = getVisibleEnd();

        if (endPos.line > getNumberLines())
            endPos.line = getNumberLines();

        unsigned line = startPos.line;

        while (line < endPos.line) {
            unsigned position = startPos.position;
            while (position < endPos.position && position < getLineLength(line)) {
                // text style end
                Pos currentPosition{line, position};
                // todo
//                const TextStyle &style = getStyleAt(currentPosition);
//                Pos styleEnd = getStyleEnd(currentPosition);
                TextStyle style(redraw, sf::Color::White, false, false, false, false);
                Pos styleEnd = getEndPos();

                std::size_t lineEnd = std::min(endPos.position, getLineLength(line));
                std::size_t partEnd = styleEnd.line == line ? std::min(styleEnd.position, lineEnd) : lineEnd;

                sf::Text text(getTextFrom({line, position}, {line, partEnd}), *font, characterSize);
                text.setFillColor(style.getTextColor());
                text.setStyle(
                        (style.isBold() ? sf::Text::Bold : 0) |
                        (style.isItalic() ? sf::Text::Italic : 0) |
                        (style.isStrikethrough() ? sf::Text::StrikeThrough : 0) |
                        (style.isUnderline() ? sf::Text::Underlined : 0)
                );
                auto drawOffset = getOffsetOf(currentPosition);
                // SFML draws text somewhat blurry if it's not aligned to an integer, floor the offset
                text.setPosition(std::floor(drawOffset.x), std::floor(drawOffset.y));
                target.draw(text, states);
                position = partEnd;
            }

            line++;
        }

        target.draw(caret, states);

        for (const std::shared_ptr<Highlight> &highlight : highlights) {
            highlight->draw(target, states);
        }

        target.draw(scrollBarManager, states);
    }

    sf::Vector2f TextBox::getContentSize() const {
        return offset + sf::Vector2f{
                (getLongestLineLength() + 0.5f) * getCharacterWidth(),
                (getNumberLines() + 0.5f) * getLineHeight()
        };
    }

    Pos TextBox::getVisibleStart() const {
        return getPositionAt(0, 0);
    }

    Pos TextBox::getVisibleEnd() const {
        return getPositionAt(getSize(), 0.5f, 0.5f);
    }

    bool TextBox::isPositionOnScreen(const Pos &position) const {
        return getVisibleStart() <= position && position < getVisibleEnd();
    }

    Pos TextBox::getRelativeCharacters(Pos pos, int characters) const {
        if (characters < 0) {
            characters = -characters;
            do {
                if (pos.position >= characters) {
                    pos.position -= characters;
                    return pos;
                }

                if (pos.line == 0) {
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

                if (pos.line == getNumberLines()) {
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

    Pos TextBox::getRelativeLine(Pos pos, int lineAmount) const {
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

    Pos TextBox::getVisibleRelativeLine(Pos pos, int lineAmount) const {
        Pos relativePosition = getRelativeLine(pos, lineAmount);
        // if getRelativeLine needs to modify the position, use that value instead
        if (relativePosition.position != pos.position) return relativePosition;

        auto line = relativePosition.line;
        float xPos = getOffsetOf(pos).x;
        auto position = getPositionAt(xPos, line * getLineHeight(), CHARACTER_ROUNDING).position;
        return {line, std::min(position, getLineLength(line))};
    }

    void TextBox::setScrollTo(const Pos &position) {
        // todo
//        sf::Vector2f positionOffset = getOffsetOf(position);
//        scrollBarManager.getHorizontalScrollBar().setScrollPercent(positionOffset.x / scrollBarManager.getHorizontalScrollBar().getMaxScrollOffset());
//        scrollBarManager.getVerticalScrollBar().setScrollPercent(positionOffset.y / scrollBarManager.getVerticalScrollBar().getMaxScrollOffset());
    }

    std::size_t TextBox::getLineLength(std::size_t line) const {
        return line == getNumberLines() ? 0 : getLine(line).getNumberCharacters();
    }

    CharPos TextBox::getCharPos(const Pos &pos) {
        if (pos == getEndPos()) return endCharPosDataHolder.getCharPos(nullptr, nullptr);

        Line &line = getLine(pos.line);
        if (pos.position == line.getNumberCharacters())
            return line.endLineCharPosDataHolder.getCharPos(&line, nullptr);

        CharInfo &info = line.getCharInfo(pos.position);
        return info.referenceHolder.getCharPos(&line, &info);
    }

    namespace detail {
        void CharPosDataHolder::transfer(const CharPos &pos) {
            if (active()) {
                reference.lock()->setRelative(pos);
                reference.reset();
            }
        }

        CharPos CharPosDataHolder::getCharPos(Line *line, CharInfo *info) {
            if (active()) return reference.lock();
            CharPos charPos = std::make_shared<CharPosData>(line == nullptr ? nullptr : line->getReference(), info);
            reference = charPos;
            return charPos;
        }

        void CharPosDataHolder::updateLine(Line &line) {
            if (active()) reference.lock()->updateLine(line.getReference());
        }

        void CharPosDataHolder::updateCharInfo(CharInfo *info) {
            if (active()) reference.lock()->updateCharInfo(info);
        }
    }

    namespace {
        void order(Pos &first, Pos &second) {
            if (second < first)
                std::swap(first, second);
        }
    }

    sf::String TextBox::getTextFrom(Pos first, Pos second) const {
        if (first == second) return "";
        order(first, second);

        if (first.line == second.line) return getLineContents(first.line, first.position, second.position);

        sf::String str;

        unsigned line = first.line;
        unsigned startIndex = first.position;

        while (line < second.line) {
            str += getLineContents(line, startIndex) + '\n';
            line++;
            startIndex = 0;
        }

        str += getLineContents(second.line, 0, second.position);
        return str;
    }

    sf::String TextBox::getLineContents(std::size_t lineNumber, std::size_t start, std::size_t end) const {
        if (lineNumber == getNumberLines()) return "";

        const Line &line = getLine(lineNumber);
        // ensure start < end
        if (end < start)
            std::swap(start, end);
        // ensure start and end within bounds
        start = std::min(start, line.getNumberCharacters());
        end = std::min(end, line.getNumberCharacters());

        sf::String str;
        while (start < end) {
            str += line.getCharInfo(start).getChar();
            start++;
        }

        return str;
    }

#define ASSERT_POSITION(pos) \
    assert((pos).line <= getNumberLines() && #pos " line out of bounds"); \
    assert((pos).position <= getLineLength((pos).line) && #pos " position out of bounds");

    Pos TextBox::insertText(Pos pos, const sf::String &text) {
        ASSERT_POSITION(pos)
        setRedrawRequired();

        std::size_t startIndex = 0;
        std::size_t endIndex = text.find('\n');

        if (endIndex != sf::String::InvalidPos) {
            if (pos.line == getNumberLines()) lines.emplace_back(this);
            Line &next = *lines.emplace(lines.begin() + pos.line + 1, this);

            Line &startLine = getLine(pos.line);
            startLine.move(next, pos.position, 0);
            startLine.insert(text.substring(0, endIndex), pos.position);

            pos.position = endIndex;
            startIndex = endIndex + 1;

            while ((endIndex = text.find('\n', endIndex + 1)) != sf::String::InvalidPos) {
                pos.position = endIndex - startIndex;
                lines.emplace(lines.begin() + ++pos.line, this)->insert(text.substring(startIndex, pos.position));
                startIndex = endIndex + 1;
            }
            pos.line++;

            return pos;
        }

        getOrInsertLine(pos.line).insert(text.substring(startIndex), pos.position);
        return {pos.line, pos.position + text.getSize() - startIndex};
    }

    Pos TextBox::insertLine(unsigned line, const sf::String &string) {
        assert(line <= getNumberLines() && "line out of bounds");
        lines.emplace(lines.begin() + line, this);
        return insertText({line, 0}, string);
    }

    void TextBox::removeText(Pos from, Pos to) {
        if (from == to) return;
        order(from, to);
        ASSERT_POSITION(from)
        ASSERT_POSITION(to)
        setRedrawRequired();

        if (from.line == to.line) {
            getLine(from.line).remove(from.position, to.position);
            return;
        }

        Line &fromLine = getLine(from.line);
        if (from.position != fromLine.getNumberCharacters())
            fromLine.remove(from.position);
        unsigned line = from.line + 1;

        removeLines(line, to.line);
        to.line = line;

        if (to.position != 0) {
            getLine(to.line).remove(0, to.position);
        }

        if (to.line != getNumberLines()) {
            Line &toLine = getLine(to.line);
            if (toLine.getNumberCharacters() > 0) {
                // move line contents from to -> from.line directly after from
                toLine.move(fromLine, 0, from.position);
            }
            removeLine(to.line);
        }
    }

    void TextBox::removeLine(unsigned int line) {
        getLine(line).prepareRemoveAll(getTransferPos(line, line + 1));
        lines.erase(lines.begin() + line);
    }

    void TextBox::removeLines(unsigned int start, unsigned int end) {
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

    bool TextBox::isOutBounds(bool verify, int x, int y) const {
        return !(verify || (offset.x <= x && x <= getSize().x && offset.y <= y && y <= getSize().y));
    }

    void TextBox::handleEvent(const sf::Event &event, bool verifyArea) {
        if (event.type == sf::Event::KeyPressed) {
            handleInput(event.key.code, true, event.key.control, event.key.shift, event.key.alt);
        } else if (event.type == sf::Event::KeyReleased) {
            handleInput(event.key.code, false, event.key.control, event.key.shift, event.key.alt);
        } else if (event.type == sf::Event::TextEntered) {
            if (inputHandler->isTextInput(event.text.unicode)) {
                sf::String string;
                // todo - remove replacing tab with spaces when support for non-monospaced characters are added
                if (event.text.unicode == '\t') string = "    ";
                    // convert carriage return to newline
                else if (event.text.unicode == '\r') string = '\n';
                else string = event.text.unicode;
                handleTextInput(string);
            }
        } else if (event.type == sf::Event::MouseWheelScrolled) {
            if (isOutBounds(verifyArea, event.mouseWheelScroll.x, event.mouseWheelScroll.y)) return;

            handleScroll(event.mouseWheelScroll.wheel == sf::Mouse::Wheel::VerticalWheel, event.mouseWheelScroll.delta);
        } else if (event.type == sf::Event::MouseButtonPressed) {
            if (isOutBounds(verifyArea, event.mouseButton.x, event.mouseButton.y)) return;

            handleInput(event.mouseButton.button, true, event.mouseButton.x, event.mouseButton.y);
        } else if (event.type == sf::Event::MouseButtonReleased) {
            // no checks to isOutBounds (mouse release is to end selection)
            handleInput(event.mouseButton.button, false, event.mouseButton.x, event.mouseButton.y);
        } else if (event.type == sf::Event::MouseMoved) {
            handleMousePositionChange(event.mouseMove.x, event.mouseMove.y);
        }
    }

    void TextBox::handleInput(sf::Keyboard::Key key, bool pressed, bool control, bool shift, bool alt) {
        inputHandler->handle(key, pressed, control, shift, alt);
    }

    void TextBox::handleTextInput(const sf::String &string) {
        caret.insert(string);
    }

    void TextBox::handleScroll(bool vertical, float amount) {
        (vertical ? scrollBarManager.getVerticalScrollBar() : scrollBarManager.getHorizontalScrollBar()).moveScroll(-amount);
    }

    void TextBox::handleInput(sf::Mouse::Button button, bool pressed, int x, int y) {
        if (scrollBarManager.getVerticalScrollBar().getScrollBarStyle().handleClick(sf::Vector2f(x, y), scrollBarManager.getVerticalScrollBar(), button, pressed) && pressed)
            return;
        if (scrollBarManager.getHorizontalScrollBar().getScrollBarStyle().handleClick(sf::Vector2f(x, y), scrollBarManager.getHorizontalScrollBar(), button, pressed) && pressed)
            return;

        if (button == sf::Mouse::Button::Left) {
            if (pressed) {
                caret.setClosestPosition(getPositionAt(x, y, CHARACTER_ROUNDING));
                selectionActive = true;
            } else {
                selectionActive = false;
            }
        }
    }

    void TextBox::handleMousePositionChange(int x, int y) {
        scrollBarManager.getVerticalScrollBar().getScrollBarStyle().handleMouseMove(sf::Vector2f(x, y), scrollBarManager.getVerticalScrollBar());
        scrollBarManager.getHorizontalScrollBar().getScrollBarStyle().handleMouseMove(sf::Vector2f(x, y), scrollBarManager.getHorizontalScrollBar());

        if (selectionActive) {
            caret.setSelectionEndClosestPosition(getPositionAt(x, y, CHARACTER_ROUNDING));
        }
    }

    std::size_t TextBox::getLongestLineLength() const {
        assert(lines.size() == lineLength.size() && "lineLength and lines have different number of elements");
        return lineLength.empty() ? 0 : (***lineLength.begin()).getNumberCharacters();
    }

    std::size_t TextBox::getLineIndex(const Line *line) const {
        assert(line != nullptr && "line is nullptr");
        // same logic as CharInfo getCharacterIndex
        return line - &*lines.begin();
    }

    detail::Line &TextBox::getOrInsertLine(std::size_t line) {
        assert(line <= getNumberLines() && "line out of bounds");
        return getNumberLines() == line ? *lines.emplace(lines.begin() + line, this) : lines[line];
    }

    namespace detail {
        void Line::remove(std::size_t start, std::size_t end) {
            auto endIndex = std::min(end, getNumberCharacters());

            CharPos transferPos;

            if (start == 0) {
                auto lineIndex = getTextBox().getLineIndex(this);
                if (lineIndex == 0) {
                    // end character if exists, or end of line
                    transferPos = getTextBox().getCharPos({lineIndex, endIndex});
                } else {
                    Line &previousLine = getTextBox().getLine(lineIndex - 1);
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
            updateLineLength();
        }

        void Line::move(Line &line, std::size_t start, std::size_t insertPosition) {
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
            line.updateLineLength();
            updateLineLength();
        }

        void Line::insert(const sf::String &string, std::size_t index) {
            assert(index <= getNumberCharacters() && "index out of bounds");
            for (char c : string) {
                // todo - optimize (will push characters back multiple times)
                characters.emplace(characters.begin() + index++, c);
            }
            updateLineLength();
        }

        void Line::prepareRemove(const CharPos &transferPos, const std::vector<CharInfo>::iterator &start,
                                 const std::vector<CharInfo>::iterator &end) {
            for (auto iter = start; iter < end; iter++) {
                iter->referenceHolder.transfer(transferPos);
            }
        }
    }
}