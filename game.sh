# Del old
if [ -f maze ]; then
	rm maze
fi

# Build new
gcc game.c -o maze -Wno-unused -lncurses

# Run
./maze