#ifndef SFML_TEXTBOX_TEXTSTYLE_HPP
#define SFML_TEXTBOX_TEXTSTYLE_HPP

namespace sftb {
    /**
     * Text styling. Each character is styled according to one TextStyle object; styles
     * do not overlap, but may span several consecutive characters or lines.
     *
     * TextStyles are created and managed by their TextBox.
     */
    struct TextStyle {
        friend class TextBox;
    private:
        std::shared_ptr<bool> redraw;
        sf::Color textColor;
        bool bold, italic;
        bool underline, strikethrough;
    public:
        TextStyle(std::shared_ptr<bool> redraw, const sf::Color &textColor, bool bold, bool italic, bool underline,
                  bool strikethrough) : redraw(std::move(redraw)), textColor(textColor), bold(bold), italic(italic),
                                        underline(underline), strikethrough(strikethrough) {}

        TextStyle &pasteFrom(const TextStyle &style) {
            textColor = style.textColor;
            bold = style.bold;
            italic = style.italic;
            underline = style.underline;
            strikethrough = style.strikethrough;
            *redraw = true;

            return *this;
        }

        [[nodiscard]] const sf::Color &getTextColor() const {
            return textColor;
        }

        TextStyle &setTextColor(const sf::Color &color) {
            textColor = color;
            *redraw = true;
            return *this;
        }

        TextStyle &setBold(bool b) {
            bold = b;
            *redraw = true;
            return *this;
        }

        [[nodiscard]] bool isBold() const {
            return bold;
        }

        TextStyle &setItalic(bool i) {
            italic = i;
            *redraw = true;
            return *this;
        }

        [[nodiscard]] bool isItalic() const {
            return italic;
        }

        TextStyle &setUnderline(bool u) {
            underline = u;
            *redraw = true;
            return *this;
        }

        [[nodiscard]] bool isUnderline() const {
            return underline;
        }

        TextStyle &setStrikethrough(bool s) {
            strikethrough = s;
            *redraw = true;
            return *this;
        }

        [[nodiscard]] bool isStrikethrough() const {
            return strikethrough;
        }
    };
}

#endif //SFML_TEXTBOX_TEXTSTYLE_HPP
