To run game use:

gcc snake.cpp -o snake $(pkg-config allegro-5 allegro_font-5 allegro_primitives-5 allegro_color-5 --libs --cflags) -lstdc++

and

./snake
