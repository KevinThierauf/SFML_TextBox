#include <utility>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include "Highlight.hpp"
#include "TextBox.hpp"

namespace sftb {
    bool Highlighter::isRangeVisible(const TextBox &box, const Pos &first, const Pos &second) {
        Pos visibleStart = box.getVisibleStart();
        Pos visibleEnd = box.getVisibleEnd();

        // visible if (first, second) overlaps with (visibleStart, visibleEnd)
        return overlaps(first, second, visibleStart, visibleEnd);
    }

    void sftb::ColorHighlighter::highlight(sf::RenderTarget &target, sf::RenderStates states, const TextBox &box, const Pos &first, const Pos &second) {
        if (!isRangeVisible(box, first, second)) return;

        sf::Vector2f offsetFirst = box.getOffsetOf(first);

        sf::RectangleShape shape;
        shape.setPosition(offsetFirst);
        shape.setFillColor(highlightColor);

        if (first.line == second.line) {
            shape.setSize({(box.getOffsetOf(second) - offsetFirst).x, box.getLineHeight()});
            target.draw(shape, states);
            return;
        }

        shape.setSize({(box.getSize() - offsetFirst).x, box.getLineHeight()});
        target.draw(shape, states);

        if (second.line - 1 > first.line) {
            std::size_t nextLine = first.line + 1;
            offsetFirst = box.getOffsetOf({nextLine, 0});
            shape.setPosition(offsetFirst);
            shape.setSize({(box.getSize() - offsetFirst).x, box.getLineHeight() * static_cast<float>(second.line - nextLine)});

            target.draw(shape, states);
        }

        offsetFirst = box.getOffsetOf({second.line, 0});
        shape.setPosition(offsetFirst);
        shape.setSize({(box.getOffsetOf(second) - offsetFirst).x, box.getLineHeight()});

        target.draw(shape, states);
    }

    Highlight::Highlight(TextBox &box, std::shared_ptr<Highlighter> highlighter, const Pos &start, const Pos &end) :
            box(box.getReference()), highlighter(std::move(highlighter)), start(box.getCharPos(start)), end(box.getCharPos(end)) {
        assert(this->highlighter != nullptr && "highlighter is nullptr");
    }

    void Highlight::draw(sf::RenderTarget &target, sf::RenderStates states) const {
        TextBox &textBox = getTextBox();
        highlighter->highlight(target, states, textBox, textBox.getPositionOfChar(start), textBox.getPositionOfChar(end));
    }

    void Highlight::setStart(const Pos &s) {
        start = getTextBox().getCharPos(s);
    }

    void Highlight::setEnd(const Pos &e) {
        end = getTextBox().getCharPos(e);
    }

    HighlightHandle::~HighlightHandle() {
        remove();
    }

    void HighlightHandle::remove() {
        if(isRemoved()) return;
        highlight->getTextBox().removeHighlight(highlight);
        highlight = nullptr;
    }
}