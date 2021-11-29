#include <SFML/Graphics.hpp>
#include <iostream>
#include <filesystem>
#include <cstdio>

#define SCREEN_WIDTH 2560
#define SCREEN_HEIGHT 1440

#define SPEED (1.f / 30.f)

#undef SAVE_FRAMES
// #define SAVE_FRAMES 1

#define MAZE_SIZE (24*6)

float win_width = std::min(SCREEN_WIDTH, SCREEN_HEIGHT) / 1.5f;
float win_height = std::min(SCREEN_WIDTH, SCREEN_HEIGHT) / 1.5f;

sf::ContextSettings settings(0, 0, 8);
sf::RenderWindow window(sf::VideoMode(win_width, win_height), "Maze!",
                        sf::Style::Titlebar | sf::Style::Close, settings);
sf::Font font;
int font_size = 28;

struct Cell
{
    int x;
    int y;
    Cell *parent;
    Cell *connected[4];
    int connected_count;
    bool is_connected;
};

const float cell_size = win_width / MAZE_SIZE;
const float cell_margin = (1.f / 10) * cell_size;

Cell cells[MAZE_SIZE * MAZE_SIZE];
// Cell *begin = &cells[MAZE_SIZE / 2 * MAZE_SIZE + MAZE_SIZE / 2];
Cell *begin = &cells[0];
Cell *current = begin;

Cell *selected = NULL;

#ifdef SAVE_FRAMES
int img_i = 0;
#endif

bool is_inside_board(int x, int y)
{
    return !(x < 0 || x >= MAZE_SIZE || y < 0 || y >= MAZE_SIZE);
}

void draw_connected_cells(int min_x, int min_y, int max_x, int max_y, sf::Color color = sf::Color::White, float margin = cell_margin)
{
    float begin_x = min_x * cell_size + margin;
    float begin_y = min_y * cell_size + margin;
    float end_x = (max_x + 1) * cell_size - margin;
    float end_y = (max_y + 1) * cell_size - margin;
    sf::RectangleShape cells_rect;
    cells_rect = sf::RectangleShape(sf::Vector2f(end_x - begin_x, end_y - begin_y));
    cells_rect.setPosition(begin_x, begin_y);
    cells_rect.setFillColor(color);
    // cells_rect.setOutlineColor(sf::Color::Red);
    // cells_rect.setOutlineThickness(1.f);
    window.draw(cells_rect);
}

void draw_cell(int x, int y, sf::Color c)
{
    sf::RectangleShape cell_rect;
    cell_rect = sf::RectangleShape(sf::Vector2f(cell_size - cell_margin * 2.f, cell_size - cell_margin * 2.f));
    cell_rect.setPosition(x * cell_size + cell_margin, y * cell_size + cell_margin);
    cell_rect.setFillColor(c);
    // cell_rect.setOutlineColor(sf::Color::Red);
    // cell_rect.setOutlineThickness(1.f);
    window.draw(cell_rect);
}

void draw_cells(Cell *from, Cell *to)
{
    if (from != NULL)
    {
        draw_connected_cells(std::min(from->x, to->x), std::min(from->y, to->y),
                             std::max(from->x, to->x), std::max(from->y, to->y));
    }

    // if (from != NULL)
    // {
    //     sf::Vertex line[] = {
    //         sf::Vertex(sf::Vector2f(from->x * cell_size + (cell_size / 2), from->y * cell_size + (cell_size / 2)), sf::Color::Red),
    //         sf::Vertex(sf::Vector2f(to->x * cell_size + (cell_size / 2), to->y * cell_size + (cell_size / 2)), sf::Color::Red)};
    //     window.draw(line, 2, sf::Lines);
    //     // printf("line (%f, %f) to (%f, %f)\n", line[0].position.x, line[0].position.y, line[1].position.x, line[1].position.y);
    //     // printf("line (%f, %f) to (%f, %f)\n", from->x, from->y, to->x, to->y);
    // }

    // draw all neighbors
    for (size_t i = 0; i < 4; i++)
    {
        Cell *next = to->connected[i];
        if (next == NULL)
            return;
        draw_cells(to, next);
    }
}

void draw_maze(void)
{
    draw_cells(NULL, begin);

    draw_cell(current->x, current->y, sf::Color::Blue);
    draw_cell(begin->x, begin->y, sf::Color::Green);
    if (selected != NULL)
    {
        draw_cell(selected->x, selected->y, sf::Color::Yellow);

        Cell *temp = selected;
        while (temp != begin)
        {
            draw_connected_cells(std::min(temp->x, temp->parent->x), std::min(temp->y, temp->parent->y),
                                 std::max(temp->x, temp->parent->x), std::max(temp->y, temp->parent->y), sf::Color(30, 30, 255), cell_margin * 3.f);
            temp = temp->parent;
        }
    }

    sf::Color neighbor_color = sf::Color(255, 0, 255, 127);
    int neighbor_count = 0;

    // up
    if (current->y - 1 >= 0)
    {
        Cell *up = &cells[(current->y - 1) * MAZE_SIZE + current->x];
        if (up->parent == NULL && up != begin)
        {
            draw_cell(up->x, up->y, neighbor_color);
            neighbor_count++;
        }
    }

    // right
    if (current->x + 1 < MAZE_SIZE)
    {
        Cell *right = &cells[current->y * MAZE_SIZE + (current->x + 1)];
        if (right->parent == NULL && right != begin)
        {
            draw_cell(right->x, right->y, neighbor_color);
            neighbor_count++;
        }
    }

    // down
    if (current->y + 1 < MAZE_SIZE)
    {
        Cell *down = &cells[(current->y + 1) * MAZE_SIZE + current->x];
        if (down->parent == NULL && down != begin)
        {
            draw_cell(down->x, down->y, neighbor_color);
            neighbor_count++;
        }
    }

    // left
    if (current->x - 1 >= 0)
    {
        Cell *left = &cells[current->y * MAZE_SIZE + (current->x - 1)];
        if (left->parent == NULL && left != begin)
        {
            draw_cell(left->x, left->y, neighbor_color);
            neighbor_count++;
        }
    }

    // sf::Text neighbor_count_text("0", font, font_size);
    // neighbor_count_text.setFillColor(sf::Color::Red);
    // neighbor_count_text.setString(std::to_string(neighbor_count));
    // sf::FloatRect bounding = neighbor_count_text.getLocalBounds();
    // neighbor_count_text.setOrigin(bounding.left + bounding.width / 2.f,
    //                                    bounding.top + bounding.height / 2.f);
    // neighbor_count_text.setPosition((current->x + .5f) * cell_size, (current->y + .5f) * cell_size);
    // window.draw(neighbor_count_text);

    // if (current->parent != NULL)
    // {
    //     sf::CircleShape parent_circle(cell_size / 2.f);
    //     parent_circle.setFillColor(sf::Color(255, 255, 0, 100));
    //     parent_circle.setPosition(current->parent->x * cell_size, current->parent->y * cell_size);
    //     window.draw(parent_circle);
    // }
}

bool gen_next(void)
{
    current->is_connected = true;
    int curr_x = current->x;
    int curr_y = current->y;
    // printf("Current is at (%d, %d)\n", curr_x, curr_y);

    Cell *neighbors[4];
    int neighbor_count = 0;
    // up
    if (curr_y - 1 >= 0)
    {
        Cell *up = &cells[(curr_y - 1) * MAZE_SIZE + curr_x];
        if (up->parent == NULL && up != begin)
        {
            neighbors[neighbor_count] = up;
            neighbor_count++;
        }
    }

    // right
    if (curr_x + 1 < MAZE_SIZE)
    {
        Cell *right = &cells[curr_y * MAZE_SIZE + (curr_x + 1)];
        if (right->parent == NULL && right != begin)
        {
            neighbors[neighbor_count] = right;
            neighbor_count++;
        }
    }

    // down
    if (curr_y + 1 < MAZE_SIZE)
    {
        Cell *down = &cells[(curr_y + 1) * MAZE_SIZE + curr_x];
        if (down->parent == NULL && down != begin)
        {
            neighbors[neighbor_count] = down;
            neighbor_count++;
        }
    }

    // left
    if (curr_x - 1 >= 0)
    {
        Cell *left = &cells[curr_y * MAZE_SIZE + (curr_x - 1)];
        if (left->parent == NULL && left != begin)
        {
            neighbors[neighbor_count] = left;
            neighbor_count++;
        }
    }

    Cell *new_curr;
    // printf("Neighbor count %d\n", neighbor_count);
    if (neighbor_count == 0)
    {
        if (current->parent != NULL)
            new_curr = current->parent;
        else
            new_curr = current;
        // fprintf(stderr, "ZERO unconnected neighbors, arrrggg!\n");
        // exit(1);
        // puts("No neighbors");
    }
    else
    {
        int chosen = rand() % neighbor_count;
        // new_curr = &cells[curr_y * MAZE_SIZE + (curr_x + 1)];
        new_curr = neighbors[chosen];
        new_curr->parent = current;
        current->connected[current->connected_count++] = new_curr;
    }
    current = new_curr;

#ifdef SAVE_FRAMES
    sf::Texture texture;
    texture.create(win_width, win_height);
    texture.update(window);

    texture.copyToImage().saveToFile(std::string("imgs/maze-") + std::to_string(img_i) + std::string(".png"));
    img_i++;
#endif

    return current->parent == NULL;
}

int main(void)
{
#ifdef SAVE_FRAMES
    std::filesystem::remove_all("imgs");
    std::filesystem::create_directory("imgs");
#endif
    
    std::filesystem::create_directory("screenshots");

    std::string font_path = "/usr/share/fonts/truetype/ubuntu/UbuntuMono-R.ttf";
    if (!font.loadFromFile(font_path))
    {
        fprintf(stderr, "Failed loading font %s", font_path.c_str());
        exit(1);
    }
    sf::Text fps_text = sf::Text("0", font, font_size);
    fps_text.setFillColor(sf::Color::Blue);

    // window.setPosition(sf::Vector2i(SCREEN_WIDTH / 2 - win_width / 2,
    //                                 SCREEN_HEIGHT / 2 - win_height / 2)); // center window to screen
    window.setPosition(sf::Vector2i(0, 360));

    for (size_t y = 0; y < MAZE_SIZE; y++)
    {
        for (size_t x = 0; x < MAZE_SIZE; x++)
        {
            Cell *cell = &cells[y * MAZE_SIZE + x];
            cell->x = x;
            cell->y = y;
            cell->is_connected = false;
            // printf("Cell at [%d, %d]\n", cell->x, cell->y);
            // for (size_t i = 0; i < 4; i++)
            //     printf("  Neighbor %i = %s\n", i, cell->connected[i] == NULL ? "NULL" : "not NULL");
        }
    }

    srand(time(NULL));
    while (!gen_next());

    sf::Clock tickClock;
    sf::Time dt;
    sf::Clock frameClock;
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            switch (event.type)
            {
            case sf::Event::Closed:
                window.close();
                break;
            case sf::Event::Resized:
                break;
            case sf::Event::MouseButtonPressed:
            {
                if (event.mouseButton.button == sf::Mouse::Button::Left)
                {
                    // printf("Mouse clicked at (%d, %d)\n", event.mouseButton.x, event.mouseButton.x);
                    int maze_x = event.mouseButton.x / cell_size;
                    int maze_y = event.mouseButton.y / cell_size;
                    if (!is_inside_board(maze_x, maze_y))
                        break;

                    Cell *clicked_cell = &cells[maze_y * MAZE_SIZE + maze_x];
                    if (clicked_cell == selected)
                        selected = NULL;
                    else if (clicked_cell->parent != NULL)
                        selected = clicked_cell;
                }
                break;
            }
            case sf::Event::MouseMoved:
            {
                if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
                {
                    // printf("Mouse moved to (%d, %d)\n", event.mouseMove.x, event.mouseMove.y);
                    int maze_x = event.mouseMove.x / cell_size;
                    int maze_y = event.mouseMove.y / cell_size;
                    if (!is_inside_board(maze_x, maze_y))
                        break;

                    Cell *clicked_cell = &cells[maze_y * MAZE_SIZE + maze_x];
                    if (clicked_cell != selected && clicked_cell->parent != NULL)
                        selected = clicked_cell;
                }
                break;
            }
            case sf::Event::KeyPressed:
            {
                if (event.key.code == sf::Keyboard::S)
                {

                    sf::Texture texture;
                    texture.create(win_width, win_height);
                    texture.update(window);

                    std::time_t now = std::time(NULL);
                    std::tm * ptm = std::localtime(&now);
                    char time_buf[32];
                    std::strftime(time_buf, sizeof time_buf, "%H:%M:%S %d-%m-%Y", ptm);
                    std::string file_name = std::string("screenshot-") + std::string(time_buf) + std::string(".png");

                    if (!texture.copyToImage().saveToFile(std::string("screenshots/") + file_name))
                        fprintf(stderr, "Error saving screenshot\n");
                    else
                        printf("Saved screenshot as '%s'\n", file_name.c_str());
                }
                break;
            }
            default:
                break;
            }
        }

        if (tickClock.getElapsedTime().asSeconds() >= SPEED)
        {
            tickClock.restart();
            // puts("\ntick");

            // for (int i = 0; i < 50; i++)
            gen_next();
        }

        dt = frameClock.restart();
        window.clear(sf::Color(20, 20, 20));

        // draw shit
        draw_maze();

        int fps = 1. / dt.asSeconds();
        fps_text.setString(std::to_string(fps));
        fps_text.setPosition(win_width - fps_text.getGlobalBounds().width, win_height - font_size);
        // window.draw(fps_text);
        window.display();
        // sf::sleep(sf::seconds(.1f));
    }

    return 0;
}