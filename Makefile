TARGET=test

LIBS= -lpthread
SRCS:=$(wildcard *.c)
OBJS:=$(patsubst %.c,%.o,$(SRCS))

all : $(TARGET)

$(TARGET): $(OBJS)
	gcc $(OBJS) -o $@ $(LIBS)


.PHONY: clean

clean:
	rm $(TARGET) $(OBJS) -rf

