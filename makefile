
uratool: ./source/main.c
	g++ -I./source -ludev -o ./uratool ./source/main.c

clean:
	rm uratool

