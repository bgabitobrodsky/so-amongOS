all:
	-cd Biblioteca && $(MAKE) all
	-cd Discordiador && $(MAKE) all
	-cd Mi_Ram_HQ && $(MAKE) all
	
clean:
	-cd Discordiador && $(MAKE) clean
	-cd I_Mongo_Store && $(MAKE) clean
	-cd Mi_Ram_HQ && $(MAKE) clean
	-cd Biblioteca && $(MAKE) clean