# List of all the board related files.
BOARDSRC = $(BOARD_PATH)/boards/BEAMIC/board.c

# Required include directories
BOARDINC = $(BOARD_PATH)/boards/BEAMIC

# Shared variables
ALLCSRC += $(BOARDSRC)
ALLINC  += $(BOARDINC)
