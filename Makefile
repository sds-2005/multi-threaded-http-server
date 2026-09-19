CXX = g++
CXXFLAGS = -O3 -Wall -pthread

# Target executable name
TARGET = my_server

# Source files
SRCS = main.cpp TcpServer.cpp

# Object files (automatically generated)
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)