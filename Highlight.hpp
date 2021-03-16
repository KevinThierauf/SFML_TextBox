#ifndef SFML_TEXTBOX_HIGHLIGHT_HPP
#define SFML_TEXTBOX_HIGHLIGHT_HPP

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Color.hpp>
#include <list>
#include "Pos.hpp"
#include "CharPos.hpp"

namespace sftb {
    class TextBox;

    class Highlighter {
        friend class Highlight;
    protected:
        [[nodiscard]] static bool isRangeVisible(const TextBox &box, const Pos &first, const Pos &second);
    public:
        virtual ~Highlighter() = default;

        virtual void highlight(sf::RenderTarget &target, sf::RenderStates states, const TextBox &box, const Pos &first, const Pos &second) = 0;
    };

    class ColorHighlighter : public Highlighter {
    private:
        sf::Color highlightColor;
    public:
        explicit ColorHighlighter(const sf::Color &highlightColor) : highlightColor(highlightColor) {}

        void highlight(sf::RenderTarget &target, sf::RenderStates states, const TextBox &box, const Pos &first, const Pos &second) override;

        [[nodiscard]] const sf::Color &getHighlightColor() const {
            return highlightColor;
        }

        void setHighlightColor(const sf::Color &color) {
            highlightColor = color;
        }
    };

    class Highlight : public sf::Drawable {
        friend class TextBox;
    private:
        TextBox *box;
        std::shared_ptr<Highlighter> highlighter;
        CharPos start, end;
        std::list<Highlight>::iterator iterator;

        Highlight(TextBox &box, std::shared_ptr<Highlighter> highlighter, const Pos &start, const Pos &end);
    protected:
        void draw(sf::RenderTarget &target, sf::RenderStates states) const override;
    public:
        Highlight(const Highlight &) = delete;
        Highlight &operator=(const Highlight &) = delete;
        Highlight(Highlight &&) = default;
        Highlight &operator=(Highlight &&) = default;

        [[nodiscard]] TextBox &getTextBox() const {
            return *box;
        }

        [[nodiscard]] const std::shared_ptr<Highlighter> &getHighlighter() const {
            return highlighter;
        }

        void setHighlighter(const std::shared_ptr<Highlighter> &h) {
            assert(h != nullptr && "h is nullptr");
            highlighter = h;
        }

        [[nodiscard]] const CharPos &getStart() const {
            return start;
        }

        void setStart(const Pos &s);

        [[nodiscard]] const CharPos &getEnd() const {
            return end;
        }

        void setEnd(const Pos &e);
    };

    class HighlightHandle {
    private:
        Highlight *highlight;
    public:
        explicit HighlightHandle(Highlight *highlight = nullptr) : highlight(highlight) {
        }

        ~HighlightHandle();

        Highlight *release() {
            Highlight *tmp = highlight;
            highlight = nullptr;
            return tmp;
        }

        void remove();

        void setHighlight(Highlight *h) {
            highlight = h;
        }

        Highlight *operator->() {
            assert(highlight != nullptr && "highlight is empty");
            return highlight;
        }

        [[nodiscard]] bool isRemoved() const {
            return highlight == nullptr;
        }

        operator bool() const {
            return isRemoved();
        }
    };
}

#endif //SFML_TEXTBOX_HIGHLIGHT_HPP
