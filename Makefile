TARGET = coinPredictor
CPPFLAGS = -I/usr/include/mysql/ -std=c++11 -lmysqlclient -lcurl -lcrypto -lpthread

OBJECTS = Main.o LSTM/Util.o LSTM/NetworkManager.o LSTM/OutputLayer.o LSTM/HiddenLayer.o LSTM/LSTM.o \
	xcoin_api.o Bithumb.o DataSaver.o CoinManager.o MySQL.o
$(TARGET): $(OBJECTS)
	$(CXX) -o $@ $^ $(CPPFLAGS)

clean :
	rm $(OBJECTS) $(TARGET)
	
Main.o: Main.cpp
LSTM/OutputLayer.o: LSTM/OutputLayer.cpp LSTM/OutputLayer.h
LSTM/HiddenLayer.o: LSTM/HiddenLayer.cpp LSTM/HiddenLayer.h
LSTM/LSTM.o: LSTM/LSTM.cpp LSTM/LSTM.h
LSTM/NetworkManager.o: LSTM/NetworkManager.cpp LSTM/NetworkManager.h
LSTM/Util.o: LSTM/Util.cpp LSTM/Util.h
xcoin_api.o: xcoin_api.cpp xcoin_api.h
Bithumb.o: Bithumb.cpp Bithumb.h
DataSaver.o: DataSaver.cpp DataSaver.h
CoinManager.o: CoinManager.cpp CoinManager.h
MySQL.o: MySQL.cpp MySQL.h
