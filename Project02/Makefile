# 定义编译器
CXX = g++

# 定义编译选项
CXXFLAGS = -Wall -g

# 定义链接选项
LDFLAGS = 

# 定义目标文件
TARGET = main

# 定义源文件
SOURCES = main.cpp MIPSSimulator.cpp Pipeline.cpp producer_consumer.cpp

# 定义对象文件
OBJECTS = $(SOURCES:.cpp=.o)

# 默认目标
all: $(TARGET)

# 链接目标
$(TARGET): $(OBJECTS)
	$(CXX) $(LDFLAGS) $(OBJECTS) -o $@

# 编译源文件
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
# 运行程序
run:
	./$(TARGET) "code.txt"
# 清理编译生成的文件
clean:
	rm -f $(OBJECTS) $(TARGET)

# 依赖关系
Depend: $(SOURCES)
	$(CXX) $(CXXFLAGS) -MM $(SOURCES) > Makefile.dep

# 包含依赖关系
-include Makefile.dep