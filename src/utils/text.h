#pragma once

#include <fmt/core.h>

#include "SFML/Graphics.hpp"
#include "vectors.h"

/**** Text ******/

sf::Font loadFont(const std::string& path) {
    sf::Font font;
    if (!font.loadFromFile(path)) {
        throw std::runtime_error(
            "Failed to load font: " + path + "\n. Current working directory: " +
            std::filesystem::current_path().string() + "\n"
        );
    }
    return font;
}

struct TextDrawer {
    sf::Font              font;
    std::vector<sf::Text> texts;

    TextDrawer(const std::string& fontPath) : font(loadFont(fontPath)) {}

    struct Opts {
        Vec2                                                  pos;
        std::optional<uint8_t>                                size;
        std::optional<sf::Color>                              color;
        std::optional<bool>                                   centered = true;
        std::optional<std::reference_wrapper<const sf::Font>> font;
    };

    static sf::Text
    makeText(const Opts& opts, const std::string& str, const sf::Font& font) {
        sf::Text text;
        text.setFont(opts.font.value_or(font));
        text.setString(str);
        text.setCharacterSize(opts.size.value_or(12));
        text.setFillColor(opts.color.value_or(sf::Color::White));
        if (opts.centered) {
            text.setOrigin(
                (int)(text.getLocalBounds().width / 2),
                (int)(text.getLocalBounds().height / 2)
            );
        }
        text.setPosition(opts.pos);
        return std::move(text);
    }

    template <typename... Args>
    void draw(const Vec2& pos, Args&&... args) {
        this->draw({.pos = pos}, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void draw(const Opts& opts, Args&&... args) {
        std::stringstream ss;
        (ss << ... << std::forward<Args>(args));
        const std::string str =
            ss.str();  // force usage of non-template base overload
        this->draw(opts, str);
    }

    void draw(const Opts& opts, const std::string& str) {
        this->texts.push_back(makeText(opts, str, this->font));
    }

    void display(sf::RenderWindow& window) {
        for (const auto& text : this->texts) {
            window.draw(text);
        }
        this->texts.clear();
    }
};