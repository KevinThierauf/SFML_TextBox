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

    float ScrollBar::getAssociateSize() const {
        return std::max(1.0f, getComponent(!vertical, (**manager).contentSizeFunction()));
    }

    float ScrollBar::getDrawSpace() const {
        return getPrimary((**manager).drawSpaceFunction());
    }

    float ScrollBar::getScrollBarLength() const {
        float size = getSize();
        float available = getDrawSpace();
        // my head hurts
        return std::max(MIN_SCROLL_BAR_LENGTH, available / (size * size) * available);
    }

    float ScrollBar::getScrollBarPosition() const {
        return getScrollPercent() * (getDrawSpace() - getScrollBarLength());
    }

    void sftb::ScrollBar::draw(sf::RenderTarget &target, sf::RenderStates states) const {
        // only draw scroll bar if needed
        if (getDrawSpace() >= getSize()) return;

        sf::RectangleShape rectangle;
        if (vertical) {
            rectangle.setSize(sf::Vector2f(thickness, getScrollBarLength()));
            rectangle.setPosition({getAssociateSize() - thickness, getScrollBarPosition()});
        } else {
            rectangle.setSize(sf::Vector2f(getScrollBarLength(), thickness));
            rectangle.setPosition({getScrollBarPosition(), getAssociateSize() - thickness});
        }

        rectangle.setFillColor(color);
        target.draw(rectangle);
    }

    void ScrollBarManager::draw(sf::RenderTarget &target, sf::RenderStates states) const {
        target.draw(vertical);
        target.draw(horizontal);
    }
}