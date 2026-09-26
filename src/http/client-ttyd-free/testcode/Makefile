CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -g

# Link to native libwebsockets and standard system crypto/OpenSSL engines
LIBS     := -lwebsockets -lssl -lcrypto -lpthread

TARGET   := supabase_test
SRC      := main.cpp
OBJ      := $(SRC:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	@echo "[+] Linking and generating binary $(TARGET)..."
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(TARGET) $(LIBS)
	@echo "[+] Done! Run with: ./$(TARGET)"

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
