#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include "ScrollBar.hpp"

namespace sftb {
    void ScrollBar::setRedraw() {
        *(**manager).redraw = true;
    }

    float ScrollBar::getSize() const {
        return std::max(1.0f, getPrimary((**manager).contentSizeFunction()));
    }

    float ScrollBar::getAssociatedDrawSpace() const {
        return std::max(1.0f, getComponent(!vertical, (**manager).drawSpaceFunction()));
    }

    float ScrollBar::getDrawSpace() const {
        return getPrimary((**manager).drawSpaceFunction());
    }

    float ScrollBar::getScrollBarLength() const {
        float size = getSize();
        // subtract thickness of other scrollbar so they don't overlap
        float available = getDrawSpace();

        if (size <= 0 || available <= 0) return 0;

        return std::max(MIN_SCROLL_BAR_LENGTH, (available / size * available)
                                               - (vertical ? (**manager).getHorizontalScrollBar().thickness : (**manager).getVerticalScrollBar().thickness));
    }

    float ScrollBar::getScrollBarPosition() const {
        return getScrollPercent() * (getDrawSpace() - getScrollBarLength());
    }

    void sftb::ScrollBar::draw(sf::RenderTarget &target, sf::RenderStates states) const {
        // only draw scroll bar if needed
        if (getDrawSpace() >= getSize()) return;

        sf::RectangleShape rectangle;
        if (vertical) {
            rectangle.setPosition({getAssociatedDrawSpace() - thickness, getScrollBarPosition()});
            rectangle.setSize(sf::Vector2f(thickness, getScrollBarLength()));
        } else {
            rectangle.setPosition({getScrollBarPosition(), getAssociatedDrawSpace() - thickness});
            rectangle.setSize(sf::Vector2f(getScrollBarLength(), thickness));
        }

        rectangle.setFillColor(color);
        target.draw(rectangle);
    }

    void ScrollBarManager::draw(sf::RenderTarget &target, sf::RenderStates states) const {
        target.draw(vertical);
        target.draw(horizontal);
    }
}