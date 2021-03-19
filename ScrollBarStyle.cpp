#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <algorithm>
#include "ScrollBarStyle.hpp"
#include "ScrollBar.hpp"

namespace sftb {
    float ScrollBarStyle::getPrimary(const ScrollBar &scrollBar, const sf::Vector2f &vector) {
        return getComponent(scrollBar.isVertical(), vector);
    }

    float ScrollBarStyle::getContentSize(const ScrollBar &scrollBar) {
        return std::max(1.0f, getPrimary(scrollBar, scrollBar.getScrollBarManager().getContentSize()));
    }

    float ScrollBarStyle::getAssociatedDrawSpace(const ScrollBar &scrollBar) {
        return std::max(1.0f, getComponent(!scrollBar.isVertical(), scrollBar.getScrollBarManager().getDrawSpace()));
    }

    float ScrollBarStyle::getDrawSpace(const ScrollBar &scrollBar) {
        return getPrimary(scrollBar, scrollBar.getScrollBarManager().getDrawSpace());
    }

    void StandardScrollBarStyleBase::draw(sf::RenderTarget &target, sf::RenderStates states, const ScrollBar &scrollBar) const {
        // only draw scroll bar if needed
        if (getDrawSpace(scrollBar) >= getContentSize(scrollBar)) return;

        sf::RectangleShape rectangle;
        sf::FloatRect dimensions = getScrollBarDimensions(scrollBar);
        rectangle.setPosition(dimensions.getPosition());
        rectangle.setSize(dimensions.getSize());

        style(rectangle);
        target.draw(rectangle);
    }

    bool StandardScrollBarStyleBase::handleClick(const sf::Vector2f &position, ScrollBar &scrollBar, sf::Mouse::Button button, bool pressed) {
        if (button == sf::Mouse::Button::Left) {
            if (pressed) {
                if (inside(scrollBar, position)) {
                    scrollBar.setSelected(true);
                    return true;
                }
            } else {
                scrollBar.setSelected(false);
                return true;
            }
        }

        return false;
    }

    void StandardScrollBarStyleBase::handleMouseMove(const sf::Vector2f &current, ScrollBar &scrollBar) {
        if (scrollBar.isSelected()) {
            scrollBar.setScrollPercent(scrollBar.getScrollPercent() - (getComponent(scrollBar.isVertical(), previous) - getComponent(scrollBar.isVertical(), current)) /
                                                                      (getMaxScrollBarLength(scrollBar) - getScrollBarLength(scrollBar)));
        }
        previous = current;
    }

    sf::FloatRect StandardScrollBarStyleBase::getScrollBarDimensions(const ScrollBar &scrollBar) const {
        if (scrollBar.isVertical())
            return sf::FloatRect(getAssociatedDrawSpace(scrollBar) - thickness, getScrollBarPosition(scrollBar), thickness, getScrollBarLength(scrollBar));
        else return sf::FloatRect(getScrollBarPosition(scrollBar), getAssociatedDrawSpace(scrollBar) - thickness, getScrollBarLength(scrollBar), thickness);
    }

    float StandardScrollBarStyleBase::getMaxScrollBarLength(const ScrollBar &scrollBar) {
        // subtract thickness of other scrollbar so they don't overlap
        return getDrawSpace(scrollBar) - scrollBar.getOpposite().getScrollBarStyle().getReservedWidth();
    }

    float StandardScrollBarStyleBase::getScrollBarLength(const ScrollBar &scrollBar) {
        float size = getContentSize(scrollBar);
        float available = getMaxScrollBarLength(scrollBar);

        if (size <= 0 || available <= 0) return 0;

        return std::max(MIN_SCROLL_BAR_LENGTH, (available / size * available));
    }

    float StandardScrollBarStyleBase::getScrollBarPosition(const ScrollBar &scrollBar) {
        return scrollBar.getScrollPercent() * (getMaxScrollBarLength(scrollBar) - getScrollBarLength(scrollBar));
    }

    const sf::Color StandardScrollBarStyle::DEFAULT_SCROLL_BAR_COLOR = sf::Color(0, 0, 0); // NOLINT(cert-err58-cpp)

    void StandardScrollBarStyle::style(sf::RectangleShape &shape) const {
        shape.setFillColor(scrollBarColor);
    }
}