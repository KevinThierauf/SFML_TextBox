#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Clipboard.hpp>
#include <ostream>
#include <cmath>
#include "TextBox.hpp"

namespace sftb {
    namespace {
        void copySelectedOrLine(TextBox::Caret &caret) {
            if (!caret.hasSelection()) {
                caret.setPosition({caret.getPosition().line, 0});
                caret.setSelectionEndPos({caret.getPosition().line + 1, 0});
            }

            sf::Clipboard::setString(caret.getSelectedText());
        }

        void removeText(TextBox::Caret &caret, bool direction) {
            auto caretPosition = caret.getPosition();
            auto removeToPosition = caret.getTextBox().getRelative(caretPosition, direction ? -1 : 1);
            caret.getTextBox().removeText(removeToPosition, caretPosition);
            if(removeToPosition < caretPosition) caret.setPosition(removeToPosition);
        }
    }

    std::unique_ptr<InputHandler> InputHandler::standard() {
        class StandardInputHandler : public InputHandler {
        public:
            void handle(sf::Keyboard::Key key, bool pressed, bool control, bool shift, bool alt) override {
                if(!pressed) return;

                switch (key) {
                    case sf::Keyboard::A:
                        if (control) {
                            TextBox::Caret &caret = getTextBox().getPrimaryCaret();

                            caret.setPosition({0, 0});
                            caret.setSelectionEndPos(getTextBox().getEndPos());
                        }
                        break;
                    case sf::Keyboard::C:
                        if (control) {
                            // copy selection, or line
                            copySelectedOrLine(getTextBox().getPrimaryCaret());
                        }
                        break;
                    case sf::Keyboard::D:
                        if (control) {
                            getTextBox().removeLine(getTextBox().getPrimaryCaret().getPosition().line);
                        }
                        break;
                    case sf::Keyboard::F:
                        if (control) {
                            // todo - find
                        }
                        break;
                    case sf::Keyboard::R:
                        if (control) {
                            // todo - replace
                        }
                        break;
                    case sf::Keyboard::X: {
                        if (control) {
                            // cut selection, or line
                            TextBox::Caret &caret = getTextBox().getPrimaryCaret();
                            copySelectedOrLine(caret);
                            caret.removeSelection();
                        }
                    }
                        break;
                    case sf::Keyboard::Y:
                        if (control) {
                            // todo - redo
                        }
                        break;
                    case sf::Keyboard::Z:
                        if (control) {
                            // todo - undo
                        }
                        break;
                    case sf::Keyboard::Escape:
                        getTextBox().getPrimaryCaret().removeSelection();
                        break;
                    case sf::Keyboard::LBracket:
                        // todo - smart brackets
                        break;
                    case sf::Keyboard::RBracket:
                        // todo - smart brackets
                        break;
                    case sf::Keyboard::Quote:
                        // todo - smart brackets
                        break;
                    case sf::Keyboard::Backspace:
                        // remove character to left
                        removeText(getTextBox().getPrimaryCaret(), true);
                        // todo - smart brackets (ignore if highlighted)
                        break;
                    case sf::Keyboard::PageUp:
                        // todo
                        break;
                    case sf::Keyboard::PageDown:
                        // todo
                        break;
                    case sf::Keyboard::End: {
                        // move caret to end of line
                        TextBox::Caret &caret = getTextBox().getPrimaryCaret();
                        auto caretLine = caret.getPosition().line;
                        caret.setPosition({caretLine, getTextBox().getLineLength(caretLine)});
                    }
                        break;
                    case sf::Keyboard::Home: {
                        // move caret to start of line
                        TextBox::Caret &caret = getTextBox().getPrimaryCaret();
                        caret.setPosition({caret.getPosition().line, 0});
                    }
                        break;
                    case sf::Keyboard::Delete:
                        // remove character to right
                        removeText(getTextBox().getPrimaryCaret(), false);
                        break;
                    case sf::Keyboard::Left:
                        if (shift)
                            getTextBox().getPrimaryCaret().move(0, -1);
                        else getTextBox().getPrimaryCaret().moveSelectionEnd(0, -1);
                        break;
                    case sf::Keyboard::Right:
                        if (shift)
                            getTextBox().getPrimaryCaret().move(0, 1);
                        else getTextBox().getPrimaryCaret().moveSelectionEnd(0, 1);
                        break;
                    case sf::Keyboard::Up:
                        if (shift)
                            getTextBox().getPrimaryCaret().move(-1, 0);
                        else getTextBox().getPrimaryCaret().moveSelectionEnd(-1, 0);
                        break;
                    case sf::Keyboard::Down:
                        if (shift)
                            getTextBox().getPrimaryCaret().move(1, 0);
                        else getTextBox().getPrimaryCaret().moveSelectionEnd(1, 0);
                        break;
                    default:
                        break;
                }
            }
        };
        return std::make_unique<StandardInputHandler>();
    }

    TextBox::CharPos TextBox::getCharPos(std::weak_ptr<CharPosData> &reference, Line *line, CharInfo *info) {
        if (reference.expired()) return reference.lock();
        CharPos pos = std::make_shared<CharPosData>(line, info);
        reference = pos;
        return pos;
    }

    TextBox::CharPos TextBox::getCharPos(const TextBox::Pos &pos) {
        if (pos == getEndPos()) return endCharPosDataHolder.getCharPos(nullptr, nullptr);

        Line &line = getLine(pos.line);
        if (pos.position == line.getNumberCharacters())
            return line.endLineCharPosDataHolder.getCharPos(&line, nullptr);

        CharInfo &info = line.getCharInfo(pos.position);
        return info.referenceHolder.getCharPos(&line, &info);
    }

    bool TextBox::LineLengthCompare::operator()(TextBox::Line *left, TextBox::Line *right) const {
        return left->getNumberCharacters() < right->getNumberCharacters();
    }

    void TextBox::draw(sf::RenderTarget &target, sf::RenderStates states) const {
        // draw background
        sf::RectangleShape background(getSize());
        background.setFillColor(backgroundColor);
        target.draw(background);

        // draw text
        Pos startPos = getPositionAt(getHorizontalOffset(), getVerticalOffset());
        Pos endPos = startPos + getPositionAt(getSize()) + Pos{1, 1};

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

        // todo - draw caret
        // todo - draw highlight

        *redraw = false;
    }

    namespace {
        void order(TextBox::Pos &first, TextBox::Pos &second) {
            if (second < first)
                std::swap(first, second);
        }
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

#define ASSERT_POSITION(pos) \
    assert((pos).line <= getNumberLines() && #pos " line out of bounds"); \
    assert((pos).position <= getLineLength((pos).line) && #pos " position out of bounds");

    TextBox::Pos TextBox::insertText(Pos pos, const sf::String &text) {
        ASSERT_POSITION(pos)
        setRedrawRequired();

        std::size_t startIndex = 0;
        std::size_t endIndex = text.find('\n');

        if (endIndex != sf::String::InvalidPos) {
            if(pos.line == getNumberLines()) lines.emplace_back(this);
            Line &next = *lines.emplace(lines.begin() + pos.line + 1, this);

            Line &startLine = getLine(pos.line);
            startLine.move(this, next, pos.position, 0);
            startLine.insert(this, text.substring(0, endIndex), pos.position);

            pos.position = endIndex;
            startIndex = endIndex + 1;

            while ((endIndex = text.find('\n', endIndex + 1)) != sf::String::InvalidPos) {
                pos.position = endIndex - startIndex;
                insertLine(++pos.line, text.substring(startIndex, pos.position));
                startIndex = endIndex + 1;
            }
            pos.line++;

            return pos;
        }

        getOrInsertLine(pos.line).insert(this, text.substring(startIndex), pos.position);
        return {pos.line, pos.position + text.getSize() - startIndex};
    }

    TextBox::Pos TextBox::insertLine(unsigned line, const sf::String &string) {
        assert(line <= getNumberLines() && "line out of bounds");
        lines.emplace(lines.begin() + line, this);
        return insertText({line, 0}, string);
    }

    void TextBox::removeText(TextBox::Pos from, TextBox::Pos to) {
        if (from == to) return;
        order(from, to);
        ASSERT_POSITION(from)
        ASSERT_POSITION(to)
        setRedrawRequired();

        if (from.line == to.line) {
            getLine(from.line).remove(this, from.position, to.position);
            return;
        }

        getLine(from.line).remove(this, from.position);
        unsigned line = from.line + 1;

        removeLines(line, to.line);
        to.line = line;

        if (to.position != 0) {
            getLine(to.line).remove(this, 0, to.position);
        }

        if (to.line != getNumberLines()) {
            // move line contents from to -> from.line directly after from
            getLine(to.line).move(this, getLine(from.line), 0, from.position);
            removeLine(to.line);
        }
    }

    bool TextBox::isOutBounds(bool verify, int x, int y) const {
        return verify && !(offset.x <= x && x <= getSize().x) || !(offset.y <= y && y <= getSize().y);
    }

    void TextBox::handleEvent(const sf::Event &event, bool verifyArea) {
        if (event.type == sf::Event::KeyPressed) {
            handleInput(event.key.code, true, event.key.control, event.key.shift, event.key.alt);
        } else if (event.type == sf::Event::KeyReleased) {
            handleInput(event.key.code, false, event.key.control, event.key.shift, event.key.alt);
        } else if (event.type == sf::Event::TextEntered) {
            if (inputHandler->isTextInput(event.text.unicode))
                // convert carriage return to end of line
                handleTextInput(sf::String(event.text.unicode == '\r' ? '\n' : event.text.unicode));
        } else if (event.type == sf::Event::MouseWheelScrolled) {
            if (isOutBounds(verifyArea, event.mouseWheelScroll.x, event.mouseWheelScroll.y)) return;

            handleScroll(event.mouseWheelScroll.wheel == sf::Mouse::Wheel::VerticalWheel, event.mouseWheelScroll.delta);
            if (event.mouseWheelScroll.wheel == sf::Mouse::HorizontalWheel) {
                scrollBarManager.getHorizontalScrollBar().moveScroll(scrollBarManager.getSensitivityHorizontal() * event.mouseWheelScroll.delta);
            } else scrollBarManager.getVerticalScrollBar().moveScroll(scrollBarManager.getSensitivityVertical() * event.mouseWheelScroll.delta);
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
        (vertical ? scrollBarManager.getVerticalScrollBar() : scrollBarManager.getHorizontalScrollBar())
                .moveScroll(amount);
    }

    void TextBox::handleInput(sf::Mouse::Button button, bool pressed, int x, int y) {
        if (button == sf::Mouse::Button::Left) {
            if (pressed) {
                caret.setClosestPosition(getPositionAt(x, y));
                selectionActive = true;
            } else {
                selectionActive = false;
            }
        }
    }

    void TextBox::handleMousePositionChange(int x, int y) {
        if (selectionActive) {
            caret.setClosestPosition(getPositionAt(x, y));
        }
    }

    std::ostream &operator<<(std::ostream &out, const TextBox::Pos &position) {
        return out << "(" << position.line << ", " << position.position << ")";
    }
}