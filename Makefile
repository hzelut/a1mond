BUILD_DIR = .build
TARGET = $(BUILD_DIR)/a1mond
SRCS = $(shell find . -name '*.c')
OBJS = $(patsubst .%, $(BUILD_DIR)%, $(SRCS:.c=.o))
INCLUDE_DIRS = $(sort $(dir $(SRCS)))

CC = gcc
CFLAGS = -Wall -Wextra -g $(addprefix -I, $(INCLUDE_DIRS))
LDFLAGS = -lconfuse

USER = $(shell whoami)
ifeq ($(USER), root)
	BEAR =
	RM = rm
else
	BEAR = bear --
	RM = sudo rm
endif

.PHONY: all clean run build

all: build

build: clean
	$(BEAR) make $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	$(RM) -rf $(BUILD_DIR)

run: $(TARGET)
	./$(TARGET)

