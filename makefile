CC=gcc
DEPS=declarations.h
all_sources=main.c tiff_img_reader.c gif_img_writer.c
all_objs = $(all_sources:.c=.o)

my_project : $(all_objs)
	$(CC) -o my_project $(all_sources) -lm
	
%.o : %.c $(DEPS)
	$(CC) -c $(all_sources)

clean :
	rm my_project *.o
 