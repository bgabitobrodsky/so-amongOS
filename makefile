all:
	-cd comms && $(MAKE) all
	-cd Discordiador && $(MAKE) all
	-cd Mi_Ram_HQ && $(MAKE) all
	
clean:
	-cd Discordiador && $(MAKE) clean
	-cd I_Mongo_Store && $(MAKE) clean
	-cd Mi_Ram_HQ && $(MAKE) clean
	-cd comms && $(MAKE) clean

install:
	-cd comms/src && $(MAKE) install

uninstall:
	-cd comms/src && $(MAKE) uninstall