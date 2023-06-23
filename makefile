
uratool: ./source/main.cpp
	g++ -I./source -ludev -lblkid -o ./uratool ./source/main.cpp

clean:
	rm uratool

