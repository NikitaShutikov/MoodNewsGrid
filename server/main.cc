#include <News.h>
#include <drogon/HttpAppFramework.h>
#include <drogon/HttpClient.h>
#include <drogon/HttpTypes.h>
#include <drogon/drogon.h>
#include <drogon/orm/DbClient.h>
#include <drogon/orm/DbConfig.h>
#include <getopt.h>

#include <string>
#include <vector>

#include "services/NewsImporter.h"

int main(int argc, char* argv[]) {
	const char* host = std::getenv("DB_HOST");
	const char* port = std::getenv("DB_PORT");
	const char* dbName = std::getenv("DB_NAME");
	const char* user = std::getenv("DB_USER");
	const char* password = std::getenv("DB_PASSWORD");
	const char* newsUrl = std::getenv("NEWS_URL");

	newsUrl = newsUrl ? newsUrl : "https://www.vedomosti.ru/rss/articles.xml";

	drogon::orm::MysqlConfig dbConfig{
		.host = host ? host : "database",
		.port = static_cast<unsigned short>(port ? std::stoi(port) : 3306),
		.databaseName = dbName ? dbName : "mydb",
		.username = user ? user : "root",
		.password = password ? password : "298769qwe"};

	int opt;
	while ((opt = getopt(argc, argv, "s")) != -1) {
		switch (opt) {
			case 's':
				auto newsImporter = NewsImporter(dbConfig, newsUrl);
				auto news = newsImporter.fetchNews();
				if (news.empty()) {
					std::cerr << "Fetched 0 news" << std::endl;
					return 1;
				}
				newsImporter.seedNews(news);
				return 0;
		}
	}

	drogon::app().loadConfigFile("config.json");
	drogon::app().addListener("0.0.0.0", 5555);
	drogon::app().run();

	return 0;
}
