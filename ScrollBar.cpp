#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include "ScrollBar.hpp"
#include "ScrollBarStyle.hpp"

namespace sftb {
    ScrollBar::ScrollBar(ScrollBarManager **manager, bool vertical) :
            manager(manager), vertical(vertical), style(std::make_shared<StandardScrollBarStyle>(getScrollBarManager().getRedraw())) {}

    void ScrollBar::setRedraw() {
        *getScrollBarManager().redraw = true;
    }

    void sftb::ScrollBar::draw(sf::RenderTarget &target, sf::RenderStates states) const {
        style->draw(target, states, *this);
    }

    const ScrollBar &ScrollBar::getOpposite() const {
        return vertical ? getScrollBarManager().getHorizontalScrollBar() : getScrollBarManager().getVerticalScrollBar();
    }

    float ScrollBar::getMaxScrollOffset() const {
        return ScrollBarStyle::getContentSize(*this) - ScrollBarStyle::getDrawSpace(*this);
    }

    void ScrollBar::setScroll(float scroll) {
        if (scrollAmount == scroll) return;
        float previous = getScroll();
        scrollAmount = std::clamp(scroll, 0.0f, getMaxScroll());
        style->notifyScrollChange(*this, previous);
        setRedraw();
    }

    void ScrollBarManager::draw(sf::RenderTarget &target, sf::RenderStates states) const {
        target.draw(vertical);
        target.draw(horizontal);
    }
}