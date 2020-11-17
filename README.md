# SnakeGame
The classic [snake game](https://en.wikipedia.org/wiki/Snake_(video_game_genre)), implemented in C++ for my (and your) pleasure to run in the terminal.

## Compile
```
g++ src/Snake.cpp src/SnakeGame.cpp -o SnakeGame -lncurses
```

## Usage
```
./SnakeGame [ {easy|normal|hard} --esc_delay=milliseconds --sync_frame_rate ]
```
Optional parameters:
- `{easy|normal|hard}`: specify a difficulty (defaults to `normal`).  Higher difficulties have a faster frame rate!
- `--esc_delay=milliseconds` (or `-eMilliseconds`): set this to a value > 100 if the game exits unexpectedly after pressing the arrow keys.
- `--sync_frame_rate`: synchronizes the horizontal and vertical frame rates.  (By default the horizontal frame rate is faster to compensate for the difference in most font's width and heights.)

## Controls
- Movement: use the arrow keys, `aswd`, or `hjkl`.
- Pause: `space` or `p`
- Quit: `esc` or `q`
