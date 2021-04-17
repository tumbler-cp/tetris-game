#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "algorithm"
#include <iostream>
#include <vector>
#include <string>


using namespace sf;

const int width = 18;
const int height = 18;
const int total_colors = 8; // 0...7

const int offset_x = 2;
const int offset_y = 3;

bool dropping = false;
// Кол-во рядов (вертикаль)
const int rows = 20;
// Кол-во строк (горизонталь)
const int columns = 10;

// Количество заполненных линий
int lines;

// Счёт
float score;

// Уровень(Скорость игры)
int level = 1;

int field[columns][rows]; // игровое поле
int new_field[columns][rows]; // игровое поле после удаления строк
bool rows_were_deleted = false;

// Текстура спрайта
std::string current_texture = "tiles.png";

// Текстура клетки или её отсутствия
std::string current_background_texture = "newBack.png";

// Текущий тип нашей фигуры (индекс для figures)
int ftype;

// Следующий тип фигуры
int next_type = rand() % 7;

// Массив фигурок-тетрамино
int figures[7][4] =
{
	1,3,5,7, // I
	2,4,5,7, // Z
	3,5,4,6, // S
	3,5,4,7, // T
	2,3,5,7, // L
	3,5,7,6, // J
	2,3,4,5, // O
};

// Дельта Х (Куда двигаться относительно горизонтали)
int dx = 0;

// Задержка перед сдвигом фигуры вниз.
float delay = 1.0f;
// Создаем таймер
sf::Clock timer;

struct Point {
	int x;
	int y;
};

Point tetramino[4];
Point new_tetramino[4];
Point ghost[4];

bool collision_check() {
	for (int i = 0; i < 4; i++) {
		if (tetramino[i].y >= (rows - 1)) {
			return true;
		}
	}
	for (int i = 0; i < 4; i++) {
		int x = tetramino[i].x;
		int y = tetramino[i].y;
		if (field[x][y + 1] != -1) {
			return true;
		}
	}
	return false;
}
bool collision_check_m() {
	for (int i = 0; i < 4; i++) {
		if (ghost[i].y >= (rows - 1)) {
			return true;
		}
	}
	for (int i = 0; i < 4; i++) {
		int x = ghost[i].x;
		int y = ghost[i].y;
		if (field[x][y + 1] != -1) {
			return true;
		}
	}
	return false;
}

bool out_of_bounds(Point t[4]) {
	bool out_of_bound = false;
	for (int i = 0; i <= 3; i++) {
		if (t[i].x >= columns || t[i].x < 0)
		{
			out_of_bound = true;
		}
		if (t[i].y >= rows)
		{
			out_of_bound = true;
		}
	}
	return out_of_bound;
};

bool is_trying_to_replace(Point t[4]) {
	for (int i = 0; i < 4; i++) {
		int x = t[i].x;
		int y = t[i].y;
		if (field[x][y] != -1) {
			return true;
		}
	}
	return false;
}

bool game_over() {
	for (int i = 0; i < columns; i++)
	{
		if (field[i][0] != -1 && field[i][1] != ftype ) {
			return true;
		}
	}
	return false;
}

void restart() {
	for (int x = 0; x < columns; x++)
	{
		for (int y = 0; y < rows; y++)
		{
			field[x][y] = -1;
		}
	}
	lines = 0;
	level = 1;
	score = 0;
	game_over();
}

void delete_rows() {
	for (int y = 0; y < rows; y++) {
		for (int x = 0; x < columns; x++) {
			new_field[x][y] = field[x][y];
		}
	}

	for (int y = 0; y < rows; y++) {
		bool have_empty_slots = false;
		for (int x = 0; x < columns; x++) {
			if (new_field[x][y] == -1) {
				have_empty_slots = true;
			}
		}
		if (!have_empty_slots) {
			rows_were_deleted = true;
			score += 100;
			lines++;
			for (; y >= 0; y--) {
				for (int x = 0; x < columns; x++) {
					if (y == 0) {
						new_field[x][y] = -1;
					} else {
						new_field[x][y] = new_field[x][y - 1];
					}
				}
			}
		}
	}
}

void rotate() {
	Point XO = tetramino[1];
	Point new_tetramino[4];
	for (int i = 0; i <= 3; i++) {
		int new_x = XO.x - tetramino[i].y + XO.y;
		int new_y = XO.y + tetramino[i].x - XO.x;
		new_tetramino[i].x = new_x;
		new_tetramino[i].y = new_y;
	}
	if (!out_of_bounds(new_tetramino) 
		&& !is_trying_to_replace(new_tetramino)) {
		for (int i = 0; i <= 3; i++) {
			tetramino[i] = new_tetramino[i];
		}
	}
}

void move() {
	int dy = 0;
	if (timer.getElapsedTime().asSeconds() > delay) {
		dy = 1;
		if (dropping) {
			score += 1.7f;
		}
		timer.restart();
	}

	Point new_tetramino[4];
	for (int i = 0; i <= 3; i++) {
		new_tetramino[i].x = tetramino[i].x + dx;
		new_tetramino[i].y = tetramino[i].y + dy;
	}
	if (!out_of_bounds(new_tetramino) 
		&& !is_trying_to_replace(new_tetramino)) {
		for (int i = 0; i <= 3; i++) {
			tetramino[i].y = new_tetramino[i].y;
			tetramino[i].x = new_tetramino[i].x;
		}
	}
	dx = 0;
}

void generate_new_tetramino()
{
	ftype = next_type;
	for (int i = 0; i < 4; i++) {
		tetramino[i].x = new_tetramino[i].x;
		tetramino[i].y = new_tetramino[i].y;
	}
	next_type = (rand() % 7);
	for (int i = 0; i < 4; i++) {
		new_tetramino[i].x = figures[next_type][i] % 2;
		new_tetramino[i].y = (figures[next_type][i] / 2) - 1;
	}
}

void draw_field(sf::Sprite sprite, sf::RenderWindow& window) {
	delay = 0.5f;
	for (int y = 0; y < rows; y++) {
		for (int x = 0; x < columns; x++) {
			if (field[x][y] == -1)
			{
				continue;
			}
			sprite.setTextureRect(IntRect(field[x][y] * 18, 0, 18, 18));
			sprite.setPosition(x * width, y * height);
			sprite.move(0, 0);
			window.draw(sprite);
		}
	}
}

void check_level() {
	if ((score / 2000) >= level) {
		level++;
	}
	if (delay > 0.1f) {
		delay -= 0.05f;
	}
	else {
		delay -= 0.01f;
	}
}

void set_texture(Vector2i a) {
	if (a.x > 210 && a.x < 246 && a.y > 295 && a.y < 349) {
		std::cout << "Set texture 1" << std::endl;
		current_texture = "tiles.png";
	}
	else if (a.x > 260 && a.x < 296 && a.y > 295 && a.y < 349) {
		std::cout << "Set texture 2" << std::endl;
		current_texture = "tiles_custom.png";
	}
	if (a.x > 228 && a.x < 245 && a.y > 360 && a.y < 377) {
		std::cout << "Set back_texture 1" << std::endl;
		current_background_texture = "newBack_custom.png";
	}
	else if (a.x > 278 && a.x < 295 && a.y > 360 && a.y < 377) {
		std::cout << "Set back_texture 2" << std::endl;
		current_background_texture = "newBack.png";
	}
}


int main()
{
	srand(time(0));
	for(int i = 0; i < columns; i++)
	{
		std::fill(std::begin(field[i]), std::end(field[i]), -1);
	}
	RenderWindow window(VideoMode(320, 480), "Tetris", Style::Close);
	sf::Font font;
	font.loadFromFile("19957.otf");
	Texture texture;
	Texture background_texture;
	Texture background_img;
	background_img.loadFromFile("fon.png");
	Sprite img(background_img);
	img.setTextureRect(IntRect(0, 0, 320, 480));

	// Главный спрайт
	Sprite sprite(texture);
	Sprite background(background_texture);


	// Спрайт для показа следующй фигуры
	Sprite next(background_texture);
	Sprite next_sprite(texture);
	next.setTextureRect(IntRect(0, 0, 18, 18));
	background.setTextureRect(IntRect(0, 0, 18, 18));

	// Спрайты выбора текстур
	Texture texture1;
	texture1.loadFromFile("texture1.png");
	Sprite texture1_sp(texture1);
	texture1_sp.setPosition(210.0f, 295.0f);

	Texture texture2;
	texture2.loadFromFile("texture2.png");
	Sprite texture2_sp(texture2);
	texture2_sp.setPosition(260.0f, 295.0f);

	Texture back1;
	back1.loadFromFile("newBack_custom.png");
	Sprite back1_sp(back1);
	back1_sp.setPosition(228.0f, 360.0f);

	Texture back2;
	back2.loadFromFile("newBack.png");
	Sprite back2_sp(back2);
	back2_sp.setPosition(278.0f, 360.0f);

	Text texture_t("Textures\n(click)", font);
	texture_t.setCharacterSize(25);
	texture_t.setStyle(sf::Text::Bold);
	texture_t.setFillColor(sf::Color::White);
	texture_t.setPosition(200.0f, 222.0f);
	texture_t.setOutlineColor(Color::Black);
	texture_t.setOutlineThickness(1.0f);

	// Текст окончания игры
	Text text("GAME OVER\npress\nRETURN\nto restart", font);
	text.setStyle(sf::Text::Bold);
	text.setFillColor(sf::Color::Red);
	text.setPosition(100.0f, 130.0f);
	text.setOutlineColor(Color::Black);
	text.setOutlineThickness(2.0f);

	// Тексты интерфейса
	// следующая фигура
	Text nex("Next\nTetramino:", font);
	nex.setCharacterSize(25);
	nex.setStyle(sf::Text::Bold);
	nex.setFillColor(sf::Color::White);
	nex.setPosition(200.0f, 0.0f);
	nex.setOutlineColor(Color::Black);
	nex.setOutlineThickness(1.0f);

	// линии
	Text line_c(std::string("Lines:  " + std::to_string(lines)), font);
	line_c.setCharacterSize(25);
	line_c.setStyle(sf::Text::Bold);
	line_c.setFillColor(sf::Color::White);
	line_c.setPosition(200.0f, 135.0f);
	line_c.setOutlineColor(Color::Black);
	line_c.setOutlineThickness(1.0f);

	// счёт
	Text score_c(std::string("Score:  " + std::to_string(int(score))), font);
	score_c.setCharacterSize(25);
	score_c.setStyle(sf::Text::Bold);
	score_c.setFillColor(sf::Color::White);
	score_c.setPosition(200.0f, 160.0f);
	score_c.setOutlineColor(Color::Black);
	score_c.setOutlineThickness(1.0f);


	// уровень
	Text level_c(std::string("Level " + std::to_string(int(level))), font);
	level_c.setCharacterSize(25);
	level_c.setStyle(sf::Text::Bold);
	level_c.setFillColor(sf::Color::White);
	level_c.setPosition(200.0f, 185.0f);
	level_c.setOutlineColor(Color::Black);
	level_c.setOutlineThickness(1.0f);
	

	// Звуки для игры
	// Звук удара
	SoundBuffer touch;
	touch.loadFromFile("touch.ogg");
	Sound touch_s(touch);
	// Звук удаления линии
	SoundBuffer line_b;
	line_b.loadFromFile("line.ogg");
	Sound line_s(line_b);
	// Звук вращения
	SoundBuffer rotate_b;
	rotate_b.loadFromFile("rotate.ogg");
	Sound rotate_s(rotate_b);
	// Звук окончания игры
	SoundBuffer game_over_b;
	game_over_b.loadFromFile("game_over.ogg");
	Sound game_over_s(game_over_b);
	generate_new_tetramino();
	while (window.isOpen())
	{
		background_texture.loadFromFile(current_background_texture);
		texture.loadFromFile(current_texture);
		game_over();
		check_level();
		//tetraghost();
		for (int i = 0; i < 4; i++) {
			int x = new_tetramino[i].x;
			int y = new_tetramino[i].y;
			next_sprite.setPosition(x * width, y * height);
			next_sprite.move(198, 36);
			window.draw(next_sprite);
		}
		Event event;
		while (window.pollEvent(event))
		{
			if (event.type == Event::Closed)
				window.close();

			if (event.type == Event::KeyPressed)
			{
				if (event.key.code == Keyboard::Down) {
					dropping = true;
					delay = 0.05f;
				}
			}
			
			if (event.type == Event::KeyReleased) {
				if (event.key.code == Keyboard::Up) {
					rotate_s.play();
					rotate();
				}
				if (event.key.code == Keyboard::Down) {
					dropping = false;
					delay = 0.5f;
				}
				if (event.key.code == Keyboard::Return) {
					restart();
				}
			}

			if (Keyboard::isKeyPressed(Keyboard::Left))
			{
				dx = -1;
			}
			else if (Keyboard::isKeyPressed(Keyboard::Right))
			{
				dx = 1;
			}
			if (Mouse::isButtonPressed(Mouse::Left)) {
				auto a = Mouse::getPosition(window);
				std::cout << a.x << "  " << a.y << std::endl;
				set_texture(a);
			}

		}

		window.clear(Color::Yellow);
		//________________________________________________________
		window.draw(img);
		next.move(230, 65);
		window.draw(next);
		for (int y = 0; y < rows; y++) {
			for (int x = 0; x < columns; x++) {
				background.setPosition(width * x, height * y);
				window.draw(background);
			}
		}
		if (!game_over()) {
			move();
			if (collision_check()) {
				touch_s.play();
				for (int i = 0; i < 4; i++) {
					int x = tetramino[i].x;
					int y = tetramino[i].y;
					field[x][y] = ftype;
				}
				generate_new_tetramino();
				delete_rows();
				next_sprite.setTextureRect(IntRect(next_type * 18, 0, 18, 18));
				for (int y = 0; y < 4; y++) {
					for (int x = 0; x < 2; x++) {
						next.setPosition(width * x + 224, height * y + 64);
						window.draw(next);
					}
				}
				for (int i = 0; i < 4; i++) {
					int x = new_tetramino[i].x;
					int y = new_tetramino[i].y;
					next_sprite.setPosition(x * width, y * height);
					next_sprite.move(224, 82);
					window.draw(next_sprite);
				}
				window.draw(nex);
				window.draw(line_c);
				window.draw(score_c);
				window.draw(level_c);
			}
		}
		draw_field(sprite, window);
		if (rows_were_deleted) {
			line_s.play();
			window.display();
			rows_were_deleted = false;
			for (int y = 0; y < rows; y++) {
				for (int x = 0; x < columns; x++) {
					field[x][y] = new_field[x][y];
				}
			}
			

			timer.restart();
			while (timer.getElapsedTime().asSeconds() < 0.5f) {
				continue;
			}
			timer.restart();

			window.clear(Color::White);
			window.draw(background);

			draw_field(sprite, window);
		}
		//_________________________________________________________________________________________
		sprite.setTextureRect(IntRect(ftype * 18, 0, 18, 18));
		if (!game_over()) {
			for (int i = 0; i < 4; i++) {
				int x = tetramino[i].x;
				int y = tetramino[i].y;
				sprite.setPosition(x * width, y * height);
				sprite.move(0, 0);
				window.draw(sprite);
			}
		}
		for (int y = 0; y < 4; y++) {
			for (int x = 0; x < 2; x++) {
				next.setPosition(width * x + 224, height * y + 64);
				window.draw(next);
			}
		}
		next_sprite.setTextureRect(IntRect(next_type * 18, 0, 18, 18));
		for (int i = 0; i < 4; i++) {
			int x = new_tetramino[i].x;
			int y = new_tetramino[i].y;
			next_sprite.setPosition(x * width, y * height);
			next_sprite.move(224, 82);
			window.draw(next_sprite);
		}
		/*
		for (int i = 0; i < 4; i++) {
			int x = ghost[i].x;
			int y = ghost[i].y;
			ghost_sprite.setPosition(x * width, y * height);
			ghost_sprite.move(0, 0);
			window.draw(ghost_sprite);
		}
		*/
		line_c.setString("Lines:  " + std::to_string(lines));
		score_c.setString("Score:  " + std::to_string(int(score)));
		level_c.setString("Level " + std::to_string(int(level)));
		window.draw(nex);
		window.draw(line_c);
		window.draw(score_c);
		window.draw(level_c);
		window.draw(texture1_sp);
		window.draw(texture2_sp);
		window.draw(texture_t);
		window.draw(back1_sp);
		window.draw(back2_sp);
		if (game_over()) {
			text.setCharacterSize(40);
			window.draw(text);
			game_over_s.play();
		}
		window.display();
	}

	return 0;
}
