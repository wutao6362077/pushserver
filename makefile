TARGET = ./ZPushMsger
INCPATH = -I./ -I../ -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../UtilsLib -I../HiRedis/RedisClient/ -I../HiRedis/Protocol -I../openssl/include -I../jsoncpp/include -I../log4cxx/include -I../mongodb/include -I../boost/include
LIBPATH = -lpthread -L../lib_a -lHTTPUtilities -lUtils -lCommonUtilities  -L/usr/local/lib -ljsoncpp  -lssl -lcrypto -lHiRedis -ldl -llog4cxx -lapr-1 -laprutil-1 -lexpat  -lmongoclient -lboost_system -lboost_regex -lboost_thread -lrt
INCLUDE_FLAG = -include
LFLAGS = -w 
CFLAGS = $(INCLUDE_FLAG) ../PlatformHeader.h -w -g -c
CC = g++ 

SRC_EXE	  = $(wildcard ./*.cpp)
   
OBJ_EXE	  = $(patsubst %.cpp,%.o,$(SRC_EXE))
    
all:$(TARGET)
    
$(TARGET): \
	$(OBJ_EXE) \

	$(CC) $(LFLAGS) \
		$(OBJ_EXE) \
	-o $(TARGET) $(LIBPATH)

$(OBJ_EXE):$(SRC_EXE)
	$(CC) $(CFLAGS) $*.cpp -o $@ $(INCPATH) 

clean:
	rm -f $(OBJ_EXE) 
