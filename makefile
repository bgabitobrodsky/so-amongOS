all:
	-cd Discordiador && $(MAKE) all
	-cd I_Mongo_Store && $(MAKE) all
	
clean:
	-cd Discordiador && $(MAKE) clean
	-cd I_Mongo_Store && $(MAKE) clean