#ifndef SFML_TEXTBOX_TEXTBOX_HPP
#define SFML_TEXTBOX_TEXTBOX_HPP

#include <SFML/Graphics/Drawable.hpp>
#include <vector>
#include <memory>
#include <cassert>

namespace sf {
    class Font;
}

namespace sftb {
    class TextBox : public sf::Drawable {
    public:
        using Char = sf::Uint32;

    private:
        class CharInfo {
        private:
            Char c;
            std::weak_ptr<CharInfo *> self;

            inline void move() {
                auto strongReference = self.lock();
                if (strongReference) *strongReference = this;
            }

        public:
            // while copying could be implemented, there is currently no intended
            // reason to copy CharInfo. Operators are deleted for safety.
            CharInfo(const CharInfo &) = delete;
            CharInfo &operator=(const CharInfo &) = delete;

            CharInfo(CharInfo &&other) noexcept: c(other.c), self(std::move(other.self)) {
                auto strongReference = self.lock();
                if (strongReference) *strongReference = this;
            }

            CharInfo &operator=(CharInfo &&other) noexcept {
                c = other.c;
                self = std::move(other.self);
                move();

                return *this;
            }

            [[nodiscard]] Char getChar() const {
                return c;
            }

            std::shared_ptr<CharInfo *> getSelf() {
                auto strongReference = self.lock();
                if (!strongReference) {
                    strongReference = std::make_shared<CharInfo *>(this);
                    self = strongReference;
                }

                return strongReference;
            }
        };

        struct Line {
            std::vector<CharInfo> characters;
        };

        std::vector<Line> lines;
        sf::Font *font;
        mutable std::shared_ptr<bool> redraw;

        void setRedrawRequired(bool v = true) const {
            *redraw = v;
        }

    protected:
        void draw(sf::RenderTarget &target, sf::RenderStates states) const override;
    public:
        explicit TextBox(sf::Font &font, std::shared_ptr<bool> redraw = nullptr)
                : font(&font), redraw(redraw ? std::make_shared<bool>(false) : std::move(redraw)) {
        }

        [[nodiscard]] sf::Font &getFont() const {
            return *font;
        }

        void setFont(sf::Font &f) {
            font = &f;
        }

        [[nodiscard]] bool isRedrawRequired() {
            return *redraw;
        }

        void setRedrawReference(std::shared_ptr<bool> r) {
            assert(r != nullptr && "redraw reference is nullptr");
            if (*redraw) *r = true; // retain redraw status
            redraw = std::move(r);
        }

        [[nodiscard]] std::shared_ptr<bool> getRedrawReference() {
            return redraw;
        }
    };
}


#endif //SFML_TEXTBOX_TEXTBOX_HPP
