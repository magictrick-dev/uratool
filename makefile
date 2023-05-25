
uratool: ./source/main.c
	g++ -I./source -ludev -lblkid -o ./uratool ./source/main.c

clean:
	rm uratool

