# Compiler and flags
CC = gcc
CFLAGS = -I./lib/inih-r57 -Wall -Wextra

# Directory for object files
OBJDIR = obj

# Object files with directory prefix
OBJ1 = $(OBJDIR)/server.o $(OBJDIR)/ini.o
OBJ2 = $(OBJDIR)/engine.o $(OBJDIR)/ini.o

# Source files
SRC1 = server.c
SRC2 = engine.c
SRC_LIB = lib/inih-r57/ini.c

# Executable names
EXE1 = server
EXE2 = engine

# Default target
all: $(EXE1) $(EXE2)

# Server executable
$(EXE1): $(OBJ1)
	$(CC) -o $@ $^ $(CFLAGS)

# Engine executable
$(EXE2): $(OBJ2)
	$(CC) -o $@ $^ $(CFLAGS)

# Rule to compile object files
$(OBJDIR)/%.o: %.c
	@mkdir -p $(OBJDIR)
	$(CC) -c -o $@ $< $(CFLAGS)

$(OBJDIR)/ini.o: $(SRC_LIB)
	@mkdir -p $(OBJDIR)
	$(CC) -c -o $@ $< $(CFLAGS)

# Dependencies
$(OBJDIR)/server.o: $(SRC1) $(SRC_LIB)
$(OBJDIR)/engine.o: $(SRC2) $(SRC_LIB)
$(OBJDIR)/ini.o: $(SRC_LIB)

# Clean up
clean:
	rm -rf $(OBJDIR) $(EXE1) $(EXE2)
