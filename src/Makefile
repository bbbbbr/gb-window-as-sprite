AS = $(GBDK_HOME)/bin/lcc -c
CC = $(GBDK_HOME)/bin/lcc -Wa-l -Wl-m

BIN = nyancassette.gb
OBJS = main.o

all: $(BIN)

%.s: %.ms
	maccer -o $@ $<

$(BIN): $(OBJS)
	$(CC) -o $(BIN) $(OBJS)

clean:
	rm -rf $(BIN) $(OBJS) *~

run:
	$(BGB) $(BIN)