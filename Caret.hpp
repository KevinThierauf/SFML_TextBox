#ifndef SFML_TEXTBOX_CARET_HPP
#define SFML_TEXTBOX_CARET_HPP

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/System/String.hpp>
#include <memory>
#include <cassert>
#include "Pos.hpp"
#include "CaretStyle.hpp"
#include "CharPos.hpp"
#include "Highlight.hpp"

namespace sftb {
    class TextBox;

    // todo - multiple caret support
    class Caret : public sf::Drawable {
        friend class TextBox;
    private:
        TextBox **reference;
        CharPos pos, selectionEndPos;
        std::shared_ptr<CaretStyle> style;
        HighlightHandle selectedTextHighlight;

        explicit Caret(TextBox &box, Pos position = {});

        Caret(Caret &&) = default;
        Caret &operator=(Caret &&) = default;

        [[nodiscard]] Pos getClosestPos(const Pos &position) const;
    protected:
        void draw(sf::RenderTarget &target, sf::RenderStates states) const override {
            style->draw(target, states, *this);
        }

    public:
        Caret(const Caret &) = delete;
        Caret &operator=(const Caret &) = delete;

        TextBox &getTextBox() {
            return **reference;
        }

        [[nodiscard]] const TextBox &getTextBox() const {
            return **reference;
        }

        [[nodiscard]] std::shared_ptr<CaretStyle> getCaretStyle() const {
            return style;
        }

        void setCaretStyle(std::shared_ptr<CaretStyle> s) {
            assert(s != nullptr && "s is nullptr");
            style = std::move(s);
        }

        [[nodiscard]] Pos getPosition() const;
        void setPosition(const Pos &position);

        void setClosestPosition(const Pos &position) {
            setPosition(getClosestPos(position));
        }

        [[nodiscard]] bool hasSelection() const {
            return selectionEndPos != nullptr;
        }

        [[nodiscard]] Pos getSelectionEndPos() const;
        void setSelectionEndPos(const Pos &position);

        void setSelectionEndClosestPosition(const Pos &position) {
            setSelectionEndPos(getClosestPos(position));
        }

        void removeSelection() {
            selectionEndPos = nullptr;
            selectedTextHighlight.remove();
        }

        void removeSelectedText();

        [[nodiscard]] sf::String getSelectedText() const;
        void insert(const sf::String &string);
    };
}

#endif //SFML_TEXTBOX_CARET_HPP
