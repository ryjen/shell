CC = gcc
CFLAGS = -I. -std=c11 -ggdb -W -Wall -Wvla -Werror -pedantic

DEPS = parser.h command.h io.h
LIBS = 

PROGS = shell 
TEST = test

ODIR = obj
BDIR = bin

_PROG_OBJS = command.o config.o parser.o main.o io.o
PROG_OBJS = $(patsubst %,$(ODIR)/%,$(_PROG_OBJS))

_TEST_OBJS = 
TEST_OBJS = $(patsubst %,$(ODIR)/%,$(_TEST_OBJS))

all: $(ODIR) $(BDIR) $(PROGS)

help:
	@echo "Commands: all help init $(PROG) $(TEST) clean"

$(ODIR):
	@[ -d $(ODIR) ] || mkdir -p $(ODIR)

$(BDIR):
	@[ -d $(BDIR) ] || mkdir -p $(BDIR)

$(PROGS): $(PROG_OBJS)
	@echo "Linking $@"
	@$(CC) -o $(BDIR)/$@ $^ $(CFLAGS) $(LIBS)

$(TEST): $(TEST_OBJS)
	@echo "Linking $@"
	@$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

$(ODIR)/%.o: %.c $(DEPS)
	@echo "Compiling $@"
	@$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: clean

clean:
	@rm -rf $(ODIR)
	@rm -f *~ core $(PROGS) $(TEST)
	@echo "Cleaned"

