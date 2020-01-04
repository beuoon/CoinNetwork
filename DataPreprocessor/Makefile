TARGET = dataPreprocessor
CPPFLAGS = -I/usr/include/mysql/ -std=c++11 -lmysqlclient

OBJECTS = dataPreprocessor.o ../MySQL.o
$(TARGET): $(OBJECTS)
	$(CXX) -o $@ $^ $(CPPFLAGS)

clean :
	rm $(OBJECTS) $(TARGET)
	
dataPreprocessor.o: dataPreprocessor.cpp
