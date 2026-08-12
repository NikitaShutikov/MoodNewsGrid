#include <drogon/HttpAppFramework.h>
#include <drogon/drogon.h>
#include <getopt.h>

int main(int argc, char* argv[]) {
	int opt;
	while ((opt = getopt(argc, argv, "hs:")) != -1) {
		switch (opt) {
			// Заполнение бд новостями
			case 's':

				return 0;
		}
	}

	drogon::app().addListener("0.0.0.0", 5555);

	// Load config file
	// drogon::app().loadConfigFile("../config.json");
	// drogon::app().loadConfigFile("../config.yaml");
	// Run HTTP framework,the method will block in the internal event loop
	drogon::app().run();
	return 0;
}
