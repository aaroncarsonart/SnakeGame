# SnakeGame
The classic [snake game](https://en.wikipedia.org/wiki/Snake_(video_game_genre)), implemented in C++ for my (and your) pleasure to run in the terminal.

## Compile
```
g++ -std=c++17 src/Snake.cpp src/SnakeGame.cpp -lncurses -o SnakeGame
```

## Usage
```
./SnakeGame [ {easy|normal|hard} -dhs -eMilliseconds ]
```

### Optional parameters
- `{easy|normal|hard}`: specify a difficulty (defaults to `normal`).  Higher difficulties have a faster frame rate!
- `--disable_colors` (`-d`) disables color output.
- `--esc_delay=milliseconds` (`-eMilliseconds`): set this to a value > 100 if the game exits unexpectedly after pressing the arrow keys.
- `--help` (`-h`) displays help info.
- `--sync_frame_rate` (`-s`) synchronizes the horizontal and vertical frame rates.  (By default the horizontal frame rate is faster to compensate for the difference in most font's width and heights.)

### Example
To play the game on easy with colors disabled:
```
./SnakeGame easy -d
```

## Controls
- __Movement:__ use the arrow keys, `aswd`, or `hjkl`.
- __Pause:__ `space` or `p`
- __Quit:__ `esc` or `q`
