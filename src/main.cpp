#include <flecs.h>

#include <SFML/Graphics.hpp>
#include <iostream>

sf::View initWindow(sf::RenderWindow& window) {
  window.setFramerateLimit(144);
  sf::Vector2u windowSize = window.getSize();
  sf::View view;
  view.setCenter(0, 0);
  view.setSize(static_cast<float>(windowSize.x),
               static_cast<float>(windowSize.y));
  window.setView(view);
  return view;
}

int tiles[16] = {
    0, 1, 1, 0,  // don't format
    1, 1, 1, 1,  // don't format
    1, 1, 1, 1,  // don't format
    0, 1, 1, 0   // don't format
};

int main() {
  auto window = sf::RenderWindow{{1920u, 1080u}, "Asteroids"};
  sf::View view = initWindow(window);

  flecs::world ecs;

  ecs.component<sf::Vector2f>("Position");
  ecs.entity("Player").set<sf::Vector2f>({1, 1});

  while (window.isOpen()) {
    window.clear(sf::Color::Black);

    for (auto event = sf::Event{}; window.pollEvent(event);) {
      switch (event.type) {
        case sf::Event::Closed:
          window.close();
          break;
        case sf::Event::Resized:
          view.setSize(static_cast<float>(event.size.width),
                       static_cast<float>(event.size.height));
          window.setView(view);
          break;
        case sf::Event::KeyPressed:
          if (event.key.code == sf::Keyboard::Escape) {
            window.close();
          }
          break;
        default:
          break;
      }
    }

    for (int i = 0; i < 16; i++) {
      const int x = i % 4;
      const int y = i / 4;

      sf::RectangleShape tile;
      tile.setSize({100, 100});
      tile.setPosition(x * 100, y * 100);
      if (tiles[i] == 1) {
        tile.setFillColor(sf::Color::Green);
      } else {
        tile.setFillColor(sf::Color::Blue);
      }
      window.draw(tile);
    }

    ecs.each([&window](flecs::entity e, sf::Vector2f& pos) {
      pos.x += 0.01;

      sf::RectangleShape player;
      player.setSize({100, 100});
      player.setPosition(pos);
      player.setFillColor(sf::Color::Red);
      window.draw(player);
    });

    window.display();
  }

  // std::cout << "Hello, World!" << std::endl;
}