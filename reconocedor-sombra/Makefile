

#CC = gcc -ggdb

CC = gcc 


OBJ = busca-sombra.o jpeg.o bits.o extra.o ltqnorm.o 

all: sombra

clean:
			rm -f *.o *% core sombra

sombra: $(OBJ) 
			$(CC) -o sombra $(OBJ) 

jpeg.o: jpeg.h
bits.o: bits.h
ltqnorm.o: ltqnorm.h
busca-sombra.o: jpeg.h
extra.o: ppm.h

