debug:
	clear
	@echo "building debug..."
	g++ -Isrc -Iinc -O0 -o bin/debug/othello src/*.cpp -lsfml-graphics -lsfml-window -lsfml-system -lGL -std=c++23
	./bin/debug/othello

release:
	clear
	@echo "building release..."
	g++ -Isrc -Iinc -O3 -o bin/release/othello src/*.cpp -lsfml-graphics -lsfml-window -lsfml-system -lGL -fexpensive-optimizations -std=c++23
	./bin/release/othello