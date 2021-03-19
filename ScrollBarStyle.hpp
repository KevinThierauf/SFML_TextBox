#ifndef SFML_TEXTBOX_SCROLLBARSTYLE_HPP
#define SFML_TEXTBOX_SCROLLBARSTYLE_HPP

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Window/Mouse.hpp>
#include <memory>
#include <utility>

namespace sf {
    class RenderTarget;
    class RectangleShape;
}

namespace sftb {
    class ScrollBar;

    class ScrollBarStyle {
        friend class ScrollBar;
        friend class TextBox;
    protected:
        virtual void draw(sf::RenderTarget &target, sf::RenderStates states, const ScrollBar &scrollBar) const = 0;

        virtual void notifyScrollChange(const ScrollBar &scrollBar, float previousScroll) {
        }

        // returns true to consume event (prevent other elements from handling this click)
        virtual bool handleClick(const sf::Vector2f &position, ScrollBar &scrollBar, sf::Mouse::Button button, bool pressed) = 0;
        virtual void handleMouseMove(const sf::Vector2f &position, ScrollBar &scrollBar) = 0;

        [[nodiscard]] static float getComponent(bool component, const sf::Vector2f &vector) {
            return component ? vector.y : vector.x;
        }

        [[nodiscard]] static float getPrimary(const ScrollBar &scrollBar, const sf::Vector2f &vector);

        [[nodiscard]] static float getContentSize(const ScrollBar &scrollBar);
        [[nodiscard]] static float getDrawSpace(const ScrollBar &scrollBar);
        [[nodiscard]] static float getAssociatedDrawSpace(const ScrollBar &scrollBar);
    public:
        virtual ~ScrollBarStyle() = default;

        // used so that this scroll bar won't overlap with the other (vertical/horizontal) scrollbar
        [[nodiscard]] virtual float getReservedWidth() const = 0;
    };

    class StandardScrollBarStyleBase : public ScrollBarStyle {
    public:
        static constexpr float DEFAULT_SCROLL_BAR_THICKNESS = 12.0f;
        static constexpr float MIN_SCROLL_BAR_LENGTH = 20.0f;
    private:
        std::shared_ptr<bool> redraw;
        float thickness;
        sf::Vector2f previous;
    protected:
        void draw(sf::RenderTarget &target, sf::RenderStates states, const ScrollBar &scrollBar) const override;
        bool handleClick(const sf::Vector2f &position, ScrollBar &scrollBar, sf::Mouse::Button button, bool pressed) override;
        void handleMouseMove(const sf::Vector2f &position, ScrollBar &scrollBar) override;
        virtual void style(sf::RectangleShape &shape) const = 0;
    public:
        explicit StandardScrollBarStyleBase(std::shared_ptr<bool> redraw, float thickness = DEFAULT_SCROLL_BAR_THICKNESS) : redraw(std::move(redraw)), thickness(thickness) {}

        void setRedraw() {
            *redraw = true;
        }

        [[nodiscard]] static float getMaxScrollBarLength(const ScrollBar &scrollBar);
        [[nodiscard]] static float getScrollBarLength(const ScrollBar &scrollBar);
        [[nodiscard]] static float getScrollBarPosition(const ScrollBar &scrollBar);
        [[nodiscard]] sf::FloatRect getScrollBarDimensions(const ScrollBar &scrollBar) const;

        virtual bool inside(const ScrollBar &scrollBar, const sf::Vector2f &vector) {
            return getScrollBarDimensions(scrollBar).contains(vector);
        }

        [[nodiscard]] float getReservedWidth() const override {
            return thickness;
        }

        [[nodiscard]] float getThickness() const {
            return thickness;
        }

        void setThickness(float t) {
            thickness = t;
            setRedraw();
        }
    };

    class StandardScrollBarStyle : public StandardScrollBarStyleBase {
    public:
        static const sf::Color DEFAULT_SCROLL_BAR_COLOR;
    private:
        sf::Color scrollBarColor;
    protected:
        void style(sf::RectangleShape &shape) const override;
    public:
        explicit StandardScrollBarStyle(const std::shared_ptr<bool> &redraw, const sf::Color &scrollBarColor = DEFAULT_SCROLL_BAR_COLOR,
                                        float thickness = DEFAULT_SCROLL_BAR_THICKNESS) : StandardScrollBarStyleBase(redraw, thickness), scrollBarColor(scrollBarColor) {}

        [[nodiscard]] const sf::Color &getScrollBarColor() const {
            return scrollBarColor;
        }

        void setScrollBarColor(const sf::Color &color) {
            scrollBarColor = color;
            setRedraw();
        }
    };
}

#endif //SFML_TEXTBOX_SCROLLBARSTYLE_HPP
