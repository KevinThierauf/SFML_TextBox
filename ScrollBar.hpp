#ifndef SFML_TEXTBOX_SCROLLBAR_HPP
#define SFML_TEXTBOX_SCROLLBAR_HPP

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Color.hpp>
#include <functional>
#include <utility>
#include "Reference.hpp"

namespace sftb {
    constexpr float MIN_SCROLL_BAR_LENGTH = 20;
    class ScrollBarManager;

    class ScrollBar : public sf::Drawable {
    private:
        ScrollBarManager *manager;
        bool vertical;
        float scrollAmount = 0;
        sf::Color color = sf::Color::Black;
        float thickness = 40.0f;

        [[nodiscard]] float getComponent(bool component, const sf::Vector2f &vector) const {
            return component ? vector.y : vector.x;
        }

        [[nodiscard]] float getPrimary(const sf::Vector2f &vector) const {
            return getComponent(vertical, vector);
        }

        void setRedraw();

        [[nodiscard]] float getSize() const;
        [[nodiscard]] float getAssociateSize() const;
        [[nodiscard]] float getDrawSpace() const;

        [[nodiscard]] float getScrollBarLength() const;
        [[nodiscard]] float getScrollBarPosition() const;
    protected:
        void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

    public:
        ScrollBar(ScrollBarManager *manager, bool vertical) :
                manager(manager), vertical(vertical) {}

        [[nodiscard]] bool isVertical() const {
            return vertical;
        }

        [[nodiscard]] float getMaxOffset() const {
            return std::max(0.0f, getSize() - getDrawSpace());
        }

        [[nodiscard]] float getScrollOffset() const {
            // scroll may go beyond what is available (for example, if the scrolled content shrinks)
            // in this event, keep the out of bounds scroll, but return a value within bounds
            // out of bounds value is preserved in case the previous size is restored
            // when the scroll is changed, the scroll's ability to be out of bounds is reset
            return std::min(scrollAmount, getMaxOffset());
        }

        [[nodiscard]] float getScrollPercent() const {
            return std::min(getScrollOffset() / getMaxOffset(), 1.0f);
        }

        void setScroll(float scroll) {
            scrollAmount = std::clamp(scroll, 0.0f, getMaxOffset());
            setRedraw();
        }

        void moveScroll(float amount) {
            setScroll(getScrollOffset() + amount);
        }

        void setScrollPercent(float percent) {
            setScroll(percent * getMaxOffset());
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
    };

    class ScrollBarManager : public sf::Drawable, public Reference<ScrollBarManager> {
        friend ScrollBar;
    public:
        static constexpr float DEFAULT_SCROLL_SENSITIVITY = 40;
    private:
        std::shared_ptr<bool> redraw;
        std::function<sf::Vector2f()> contentSizeFunction;
        std::function<sf::Vector2f()> drawSpaceFunction;
        ScrollBar vertical, horizontal;
        float sensitivityVertical = DEFAULT_SCROLL_SENSITIVITY, sensitivityHorizontal = DEFAULT_SCROLL_SENSITIVITY;
    protected:
        void draw(sf::RenderTarget &target, sf::RenderStates states) const override;
    public:
        ScrollBarManager(std::shared_ptr<bool> redraw, std::function<sf::Vector2f()> drawSpaceFunction,
                         std::function<sf::Vector2f()> contentSizeFunction) :
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

        [[nodiscard]] float getSensitivityVertical() const {
            return sensitivityVertical;
        }

        void setSensitivityVertical(float sensitivity) {
            sensitivityVertical = sensitivity;
        }

        [[nodiscard]] float getSensitivityHorizontal() const {
            return sensitivityHorizontal;
        }

        void setSensitivityHorizontal(float sensitivity) {
            sensitivityHorizontal = sensitivity;
        }
    };
}

#endif //SFML_TEXTBOX_SCROLLBAR_HPP
