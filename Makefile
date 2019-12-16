TARGET = coinPredictor
CPPFLAGS = -I/usr/include/mysql/ -std=c++11 -lmysqlclient -lcurl -lcrypto -lpthread

OBJECTS = Main.o ANN/Util.o ANN/NetworkManager.o ANN/OutputLayer.o ANN/HiddenLayer.o ANN/DQRN.o \
	xcoin_api.o Bithumb.o DataSaver.o CoinManager.o MySQL.o
$(TARGET): $(OBJECTS)
	$(CXX) -o $@ $^ $(CPPFLAGS)

clean :
	rm $(OBJECTS) $(TARGET)
	
Main.o: Main.cpp
ANN/OutputLayer.o: ANN/OutputLayer.cpp ANN/OutputLayer.h
ANN/HiddenLayer.o: ANN/HiddenLayer.cpp ANN/HiddenLayer.h
ANN/DQRN.o: ANN/DQRN.cpp ANN/DQRN.h
ANN/NetworkManager.o: ANN/NetworkManager.cpp ANN/NetworkManager.h
ANN/Util.o: ANN/Util.cpp ANN/Util.h
xcoin_api.o: xcoin_api.cpp xcoin_api.h
Bithumb.o: Bithumb.cpp Bithumb.h
DataSaver.o: DataSaver.cpp DataSaver.h
CoinManager.o: CoinManager.cpp CoinManager.h
MySQL.o: MySQL.cpp MySQL.h
