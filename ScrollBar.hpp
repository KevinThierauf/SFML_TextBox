#ifndef SFML_TEXTBOX_SCROLLBAR_HPP
#define SFML_TEXTBOX_SCROLLBAR_HPP

#include <SFML/Graphics/Drawable.hpp>
#include <functional>
#include <utility>
#include "Reference.hpp"

namespace sftb {
    class ScrollBarManager;
    class ScrollBarStyle;

    // todo - smooth scrolling
    class ScrollBar : public sf::Drawable {
        friend class ScrollBarManager;
    public:
        static constexpr float DEFAULT_SCROLL_SENSITIVITY = 80;
    private:
        ScrollBarManager **manager;
        bool vertical;
        std::shared_ptr<ScrollBarStyle> style;
        float scrollAmount = 0;
        float sensitivity = DEFAULT_SCROLL_SENSITIVITY;
        bool selected = false;

        void setRedraw();

        ScrollBar(ScrollBarManager **manager, bool vertical);
    protected:
        void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

    public:
        [[nodiscard]] ScrollBarManager &getScrollBarManager() {
            return **manager;
        }

        [[nodiscard]] bool isVertical() const {
            return vertical;
        }

        [[nodiscard]] const ScrollBarManager &getScrollBarManager() const {
            return **manager;
        }

        [[nodiscard]] ScrollBarStyle &getScrollBarStyle() {
            return *style;
        }

        [[nodiscard]] const ScrollBarStyle &getScrollBarStyle() const {
            return *style;
        }

        void setScrollbarStyle(std::shared_ptr<ScrollBarStyle> s) {
            assert(s != nullptr && "s is nullptr");
            style = std::move(s);
            setRedraw();
        }

        [[nodiscard]] bool isSelected() const {
            return selected;
        }

        void setSelected(bool s) {
            selected = s;
        }

        [[nodiscard]] const ScrollBar &getOpposite() const;
        [[nodiscard]] float getMaxScrollOffset() const;

        [[nodiscard]] float getMaxScroll() const {
            return std::max(0.0f, getMaxScrollOffset() / sensitivity);
        }

        [[nodiscard]] float getScroll() const {
            // scroll may go beyond what is available (for example, if the scrolled content shrinks)
            // in this event, keep the out of bounds scroll, but return a value within bounds
            // out of bounds value is preserved in case the previous size is restored
            // when the scroll is changed, the scroll's ability to be out of bounds is reset
            return std::min(scrollAmount, getMaxScroll());
        }

        void setScroll(float scroll);

        [[nodiscard]] float getScrollOffset() const {
            return -sensitivity * getScroll();
        }

        void setScrollOffset(float scroll) {
            setScroll(scroll / -sensitivity);
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

        [[nodiscard]] const std::shared_ptr<bool> &getRedraw() const {
            return redraw;
        }

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

        [[nodiscard]] sf::Vector2f getContentSize() const {
            return contentSizeFunction();
        }

        [[nodiscard]] sf::Vector2f getDrawSpace() const {
            return drawSpaceFunction();
        }
    };
}

#endif //SFML_TEXTBOX_SCROLLBAR_HPP
