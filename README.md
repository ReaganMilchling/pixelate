# OVERVIEW

Codebase to 'pixelate' images for LEGO art. Converts colors to closest LEGO Round 1 x 1. 

# METHODOLOGY

Euclidean & redmean distance from https://en.wikipedia.org/wiki/Color_difference

LEGO Colors from https://rebrickable.com/downloads/


# COMMANDS
- cmake -DCMAKE_BUILD_TYPE=Debug .. && make
- gdb --args ./pixelate /home/reagan/Pictures/haku.jpg

- cmale .. && make
- ./pixelate /home/reagan/Pictures/haku.jpg
