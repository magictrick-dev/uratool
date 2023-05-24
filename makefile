
uratool: ./source/main.c
	g++ -L/usr/local/lib -I./source -lusb-1.0 -Wall -o ./uratool ./source/main.c

clean:
	rm urtool

