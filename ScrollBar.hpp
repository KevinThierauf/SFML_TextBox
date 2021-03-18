#ifndef SFML_TEXTBOX_SCROLLBAR_HPP
#define SFML_TEXTBOX_SCROLLBAR_HPP

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Color.hpp>
#include <functional>
#include <utility>
#include "Reference.hpp"

namespace sftb {
    class ScrollBarManager;

    class ScrollBar : public sf::Drawable {
    public:
        static constexpr float MIN_SCROLL_BAR_LENGTH = 20.0f;
        static constexpr float DEFAULT_SCROLL_BAR_THICKNESS = 8.0f;
        static constexpr float DEFAULT_SCROLL_SENSITIVITY = 40;
    private:
        ScrollBarManager **manager;
        bool vertical;
        float scrollAmount = 0;
        sf::Color color = sf::Color::Black;
        float thickness = DEFAULT_SCROLL_BAR_THICKNESS;
        float sensitivity = DEFAULT_SCROLL_SENSITIVITY;

        [[nodiscard]] float getComponent(bool component, const sf::Vector2f &vector) const {
            return component ? vector.y : vector.x;
        }

        [[nodiscard]] float getPrimary(const sf::Vector2f &vector) const {
            return getComponent(vertical, vector);
        }

        void setRedraw();

        [[nodiscard]] float getSize() const;
        [[nodiscard]] float getAssociatedDrawSpace() const;
        [[nodiscard]] float getDrawSpace() const;

        [[nodiscard]] float getScrollBarLength() const;
        [[nodiscard]] float getScrollBarPosition() const;
    protected:
        void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

    public:
        ScrollBar(ScrollBarManager **manager, bool vertical) :
                manager(manager), vertical(vertical) {}

        [[nodiscard]] bool isVertical() const {
            return vertical;
        }

        [[nodiscard]] float getMaxScroll() const {
            return std::max(0.0f, (getSize() - getDrawSpace()) / sensitivity);
        }

        [[nodiscard]] float getMaxScrollOffset() const {
            return -sensitivity * getMaxScroll();
        }

        [[nodiscard]] float getScroll() const {
            // scroll may go beyond what is available (for example, if the scrolled content shrinks)
            // in this event, keep the out of bounds scroll, but return a value within bounds
            // out of bounds value is preserved in case the previous size is restored
            // when the scroll is changed, the scroll's ability to be out of bounds is reset
            return std::min(scrollAmount, getMaxScroll());
        }

        void setScroll(float scroll) {
            scrollAmount = std::clamp(scroll, 0.0f, getMaxScroll());
            setRedraw();
        }

        [[nodiscard]] float getScrollOffset() const {
            return -sensitivity * getScroll();
        }

        void setScrollOffset(float scroll) {
            setScroll(scroll / sensitivity);
        }

        void moveScroll(float amount) {
            setScroll(getScroll() + amount);
        }

        [[nodiscard]] float getScrollPercent() const {
            return getScroll() / getMaxScroll();
        }

        void setScrollPercent(float percent) {
            setScroll(percent * getMaxScroll());
        }

        [[nodiscard]] const sf::Color &getColor() const {
            return color;
        }

        void setColor(const sf::Color &c) {
            color = c;
            setRedraw();
        }

        [[nodiscard]] float getThickness() const {
            return thickness;
        }

        void setThickness(float t) {
            thickness = t;
            setRedraw();
        }

        [[nodiscard]] float getSensitivity() const {
            return sensitivity;
        }

        void setSensitivity(float s) {
            // scrollAmount will be relative to s, instead of sensitivity
            // set scrollAmount to be at the same position as before
            scrollAmount = s * scrollAmount / sensitivity;
            sensitivity = s;
        }
    };

    class ScrollBarManager : public sf::Drawable, public Reference<ScrollBarManager> {
        friend ScrollBar;
    public:
    private:
        std::shared_ptr<bool> redraw;
        std::function<sf::Vector2f()> contentSizeFunction;
        std::function<sf::Vector2f()> drawSpaceFunction;
        ScrollBar vertical, horizontal;
    protected:
        void draw(sf::RenderTarget &target, sf::RenderStates states) const override;
    public:
        ScrollBarManager(std::shared_ptr<bool> redraw, std::function<sf::Vector2f()> contentSizeFunction, std::function<sf::Vector2f()> drawSpaceFunction) :
                redraw(std::move(redraw)), contentSizeFunction(std::move(contentSizeFunction)),
                drawSpaceFunction(std::move(drawSpaceFunction)), vertical(getReference(), true),
                horizontal(getReference(), false) {}

        [[nodiscard]] const ScrollBar &getVerticalScrollBar() const {
            return vertical;
        }

        [[nodiscard]] ScrollBar &getVerticalScrollBar() {
            return vertical;
        }

        [[nodiscard]] const ScrollBar &getHorizontalScrollBar() const {
            return horizontal;
        }

        [[nodiscard]] ScrollBar &getHorizontalScrollBar() {
            return horizontal;
        }
    };
}

#endif //SFML_TEXTBOX_SCROLLBAR_HPP
