all: bitmap.c
		gcc bitmap.c setLinkedListImp.c -m32 -g -o bmp -lm

clean:
	$(RM) bmp
