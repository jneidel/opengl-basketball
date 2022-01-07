# opengl basketball

> without any baskets

## Dependencies

[Arch](https://archlinux.org/packages/):

- cmake
- mesa (OpenGL implementation)

## Build

```sh
mkdir build
cd build
cmake ..
make
```

## Run

```sh
# in build/
./playground
```

If you do not see the target in front of you (i.e. looking some random
direction), just restart.

## How to play

Try to hit the target with the ball :)

**Keybindings:**
- `Mouse`: to look around
- `Left mouse button`: propel the ball roughly in the direction you're looking
(the force gets applied when the ball next touches a surface)
- `Enter`: Reset the ball to its starting position
- `Esc`: Close the game

## Known bugs

- Game sometimes starts not looking at -z, but some random direction.
- Once the ball sits still for a moment, no more force can be applied. And even
reseting will not work.

## Attribution

I have liberally copied from [opengl-tutorial.org](http://www.opengl-tutorial.org).
Using their tutorials as a base for this project.
