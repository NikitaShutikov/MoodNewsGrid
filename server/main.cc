#include <drogon/HttpClient.h>
#include <drogon/orm/DbClient.h>
#include <drogon/orm/DbConfig.h>

#include <vector>
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <News.h>
#include <drogon/HttpAppFramework.h>
#include <drogon/HttpTypes.h>
#include <drogon/drogon.h>
#include <getopt.h>
#include <httplib.h>
#include <tinyxml2.h>

using namespace drogon;
using namespace drogon_model::mydb;

std::vector<News> fetchNews() {
	std::vector<News> news;
	httplib::Client client("https://russian.rt.com");
	auto res = client.Get("https://russian.rt.com/rss");

	if (res && res->status == 200) {
		tinyxml2::XMLDocument doc;

		if (doc.Parse(res->body.c_str()) != tinyxml2::XML_SUCCESS) {
			std::cerr << "Failed to parse XML" << std::endl;
			return news;
		}

		tinyxml2::XMLElement* root = doc.FirstChildElement("rss");
		if (!root) {
			std::cerr << "No <rss> root element" << std::endl;
			return news;
		}

		tinyxml2::XMLElement* channel = root->FirstChildElement("channel");
		if (!channel) {
			std::cerr << "No <channel> element" << std::endl;
			return news;
		}

		tinyxml2::XMLElement* item = channel->FirstChildElement("item");
		while (item) {
			News newsItem;

			tinyxml2::XMLElement* titleElem = item->FirstChildElement("title");
			if (titleElem && titleElem->GetText()) {
				newsItem.setTitle(titleElem->GetText());
			}

			tinyxml2::XMLElement* linkElem = item->FirstChildElement("link");
			if (linkElem && linkElem->GetText()) {
				newsItem.setSourceUrl(linkElem->GetText());
			}

			tinyxml2::XMLElement* descriptionElem =
				item->FirstChildElement("description");
			if (descriptionElem && descriptionElem->GetText()) {
				newsItem.setOriginalText(descriptionElem->GetText());
			}

			news.push_back(newsItem);

			item = item->NextSiblingElement("item");
		}
	} else {
		std::cerr << "Failed to fetch news: " << res->status << std::endl;
	}

	return news;
}

int main(int argc, char* argv[]) {
	int opt;
	while ((opt = getopt(argc, argv, "s")) != -1) {
		switch (opt) {
			case 's':
				auto dbClient = drogon::orm::DbClient::newMysqlClient(
					"host=database port=3306 dbname=mydb user=root "
					"password=298769qwe",
					1);

				if (!dbClient) {
					std::cerr << "Cannot initialize database connection"
							  << std::endl;
					return 1;
				}

				auto news = fetchNews();

				auto transaction = dbClient->newTransaction();
				drogon::orm::Mapper<News> mapper(transaction);

				for (auto n : news) {
					try {
						mapper.insert(n);
						std::cout << "Added news with ID " << n.getId()
								  << std::endl;
					} catch (const drogon::orm::DrogonDbException& e) {
						std::cerr << "Failed to add news: " << e.base().what()
								  << std::endl;
						transaction->rollback();
						return 1;
					}
				}
				return 0;
		}
	}

	drogon::app().loadConfigFile("../config.json");
	drogon::app().addListener("0.0.0.0", 5555);
	drogon::app().run();

	return 0;
}
