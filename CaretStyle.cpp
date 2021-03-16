#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <cmath>
#include "CaretStyle.hpp"
#include "Caret.hpp"
#include "TextBox.hpp"

namespace sftb {
    void StandardCaretStyle::draw(sf::RenderTarget &target, sf::RenderStates states, const Caret &c) {
        sf::RectangleShape shape(getCaretSize(c));
        shape.setPosition(getCaretPosition(c));
        shape.setFillColor(getCurrentCaretColor());
        target.draw(shape, states);

        if (c.getTextBox().isPositionOnScreen(c.getPosition()))
            c.getTextBox().setRedrawRequired();
    }

    float StandardCaretStyle::getBlinkPercent() {
        auto time = clock.getElapsedTime().asMilliseconds();
        return caretBlinkWait > (time - lastPositionChange) ? 0.0f : 2 * std::abs(
                static_cast<float>(time % caretBlinkPeriod) / static_cast<float>(caretBlinkPeriod) - 0.5f);
    }

    sf::Color StandardCaretStyle::getCurrentCaretColor() {
        float percent = getBlinkPercent();
        sf::Color first = getFirstColor();
        sf::Color second = getSecondColor();

        return sf::Color(
                percent * static_cast<float>(second.r - first.r) + static_cast<float>(first.r),
                percent * static_cast<float>(second.g - first.g) + static_cast<float>(first.g),
                percent * static_cast<float>(second.b - first.b) + static_cast<float>(first.b),
                percent * static_cast<float>(second.a - first.a) + static_cast<float>(first.a)
        );
    }

    sf::Vector2f StandardCaretStyle::getCaretPosition(const Caret &c) const {
        auto caretPosition = c.getPosition();
        auto position = c.getTextBox().getOffsetOf(caretPosition);
        position.x -= caretWidth / 2;
        position.y += c.getTextBox().getLineHeight() / 8;
        return position;
    }

    sf::Vector2f StandardCaretStyle::getCaretSize(const Caret &c) const {
        return {caretWidth, c.getTextBox().getLineHeight()};
    }

    void StandardCaretStyle::notifyPositionChange(const Caret &c, const Pos &previousPosition) {
        lastPositionChange = clock.getElapsedTime().asMilliseconds();
    }
}