
CC = gcc
OUT = zbar

CFLAGS=  -g -Wall -c -DZBAR_WIN=1 -DZBAR_VID=1

INC = -I../zbar -I../decoder  -I../processor  -I../qrcode  -I../video  -I../window -I../include  -I../  

SRCS  = $(wildcard ./*.c ../zbar/*.c ../decoder/*.c ../processor/*.c ../qrcode/*.c ../video/*.c  ../window/*.c )



#LDFLAGS = -L /additional/lib/dir

LIBS=-lm -lpthread -lv4l2 -lX11 -lXv 

OBJFILES = $(SRCS:.c=.o)



$(OUT): $(OBJFILES)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(INC) $(CFLAGS) -c -o $@ $^


clean:
	rm -f $(OBJ) ${OBJFILES} $(OUT)

.PHONY: $(OUT)








