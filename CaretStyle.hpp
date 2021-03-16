#ifndef SFML_TEXTBOX_CARETSTYLE_HPP
#define SFML_TEXTBOX_CARETSTYLE_HPP

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Clock.hpp>
#include <memory>
#include "Pos.hpp"

namespace sfml {
    class RenderTarget;
}

namespace sftb {
    class Caret;
    class Highlighter;

    class CaretStyle {
        friend class Caret;
    public:
        static const sf::Color TEXT_HIGHLIGHT_COLOR;
    protected:
        virtual void draw(sf::RenderTarget &target, sf::RenderStates states, const Caret &caret) = 0;
        virtual std::shared_ptr<Highlighter> getSelectedTextHighlighter(const Caret &c);

        virtual void notifyPositionChange(const Caret &c, const Pos &previousPosition) {
        }

    public:
        CaretStyle() = default;
        CaretStyle(CaretStyle &) = delete;
        CaretStyle &operator=(CaretStyle &) = delete;
        CaretStyle(CaretStyle &&) = delete;
        CaretStyle &operator=(CaretStyle &&) = delete;

        virtual ~CaretStyle() = default;
    };

    class StandardCaretStyle : public CaretStyle {
    public:
        static constexpr float DEFAULT_CARET_WIDTH = 2;
        static constexpr int DEFAULT_CARET_BLINK_WAIT = 2000;
        static constexpr int DEFAULT_CARET_BLINK_PERIOD = 2000;
    private:
        sf::Color firstColor;
        sf::Color secondColor;
        sf::Clock clock;

        float caretWidth = DEFAULT_CARET_WIDTH;
        int caretBlinkWait = DEFAULT_CARET_BLINK_WAIT;
        int caretBlinkPeriod = DEFAULT_CARET_BLINK_PERIOD;

        sf::Int32 lastPositionChange = 0;
    protected:
        void draw(sf::RenderTarget &target, sf::RenderStates states, const Caret &caret) override;
        void notifyPositionChange(const Caret &caret, const Pos &previousPosition) override;
    public:
        explicit StandardCaretStyle(sf::Color firstColor = sf::Color::White,
                                    sf::Color secondColor = sf::Color::Transparent)
                : firstColor(firstColor), secondColor(secondColor) {}

        float getBlinkPercent();
        sf::Color getCurrentCaretColor();
        [[nodiscard]] sf::Vector2f getCaretPosition(const Caret &caret) const;
        [[nodiscard]] sf::Vector2f getCaretSize(const Caret &caret) const;

        [[nodiscard]] const sf::Color &getFirstColor() const {
            return firstColor;
        }

        void setFirstColor(const sf::Color &color) {
            firstColor = color;
        }

        [[nodiscard]] const sf::Color &getSecondColor() const {
            return secondColor;
        }

        void setSecondColor(const sf::Color &color) {
            secondColor = color;
        }
    };
}

#endif //SFML_TEXTBOX_CARETSTYLE_HPP
