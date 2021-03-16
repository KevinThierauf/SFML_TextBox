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
        Highlight(Highlight &&) = delete;
        Highlight &operator=(Highlight &&) = delete;

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

        void remove();

        [[nodiscard]] bool isRemoved() const {
            return box == nullptr;
        }
    };

    class HighlightHandle {
    private:
        Highlight *highlight;
    public:
        explicit HighlightHandle(Highlight *highlight) : highlight(highlight) {
        }

        ~HighlightHandle() {
            if (highlight)
                highlight->remove();
        }

        [[nodiscard]] Highlight *release() {
            Highlight *tmp = highlight;
            highlight = nullptr;
            return tmp;
        }

        void setHighlight(Highlight *h) {
            highlight = h;
        }

        Highlight *operator->() {
            assert(highlight != nullptr && "highlight is nullptr");
            return highlight;
        }

        [[nodiscard]] bool empty() const {
            return highlight;
        }

        [[nodiscard]] bool removed() const {
            return empty() || highlight->isRemoved();
        }

        operator bool() const {
            return removed();
        }
    };
}

#endif //SFML_TEXTBOX_HIGHLIGHT_HPP
