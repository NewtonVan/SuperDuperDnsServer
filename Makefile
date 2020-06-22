objects = main.o application.o message.o query.o resolver.o response.o \
		  server.o toolMessage.o

define GetObj
g++ -c $< -o $@
endef

SuperDuperServer : $(objects)
	@g++ $(objects) -Wall -Werror -o $@	

main.o: main.cpp application.h server.h query.h message.h SuperDuperLib.h \
 response.h resolver.h toolMessage.h
	@$(GetObj)

application.o: application.cpp application.h server.h query.h message.h \
 SuperDuperLib.h response.h resolver.h toolMessage.h
	@$(GetObj)

message.o: message.cpp message.h SuperDuperLib.h
	@$(GetObj)

query.o: query.cpp query.h message.h SuperDuperLib.h
	@$(GetObj)

resolver.o: resolver.cpp SuperDuperLib.h resolver.h query.h message.h \
 response.h toolMessage.h
	@$(GetObj)

response.o: response.cpp response.h message.h SuperDuperLib.h query.h
	@$(GetObj)

server.o: server.cpp SuperDuperLib.h server.h query.h message.h \
 response.h resolver.h toolMessage.h
	@$(GetObj)

toolMessage.o: toolMessage.cpp toolMessage.h message.h SuperDuperLib.h
	@$(GetObj)


.PHONY : clean
clean :
	@-rm $(objects)
