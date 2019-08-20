TARGET = coinPredictor
CPPFLAGS = -std=c++11
OBJECTS = Main.o LSTM/Util.o LSTM/OutputLayer.o LSTM/HiddenLayer.o LSTM/LSTM.o

$(TARGET): $(OBJECTS)
	$(CXX) $(CPPFLAGS) -o $@ $^

clean :
	rm $(OBJECTS) $(TARGET)
	
Main.o: Main.cpp
LSTM/OutputLayer.o: LSTM/OutputLayer.cpp LSTM/OutputLayer.h
LSTM/HiddenLayer.o: LSTM/HiddenLayer.cpp LSTM/HiddenLayer.h
LSTM/LSTM.o: LSTM/LSTM.cpp LSTM/LSTM.h
LSTM/Util.o: LSTM/Util.cpp LSTM/Util.h