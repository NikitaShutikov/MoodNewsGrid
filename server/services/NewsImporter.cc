#include <utility>
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <drogon/orm/DbConfig.h>
#include <httplib.h>
#include <tinyxml2.h>

#include <format>

#include "NewsImporter.h"

using namespace drogon;
using namespace drogon::orm;
using namespace drogon_model::mydb;

NewsImporter::NewsImporter(MysqlConfig dbConfig, std::string newsUrl)
	: newsUrl(std::move(newsUrl)) {
	this->dbConnInfo =
		std::format("host={} port={} dbname={} user={} password={}",
					dbConfig.host, dbConfig.port, dbConfig.databaseName,
					dbConfig.username, dbConfig.password);
}

std::vector<News> NewsImporter::fetchNews() {
	std::vector<News> news;
	httplib::Client client(newsUrl);
	auto res = client.Get(newsUrl);

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

			// TODO: Обработать ситуации когда нету полей title, link или
			// description
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
				news.push_back(newsItem);
			} else {
				std::cerr << "Missing description for news" << std::endl;
			}

			item = item->NextSiblingElement("item");
		}
	} else {
		std::cerr << "Failed to fetch news: " << res->status << std::endl;
	}

	return news;
}

bool NewsImporter::seedNews(std::vector<News> news) {
	auto dbClient = drogon::orm::DbClient::newMysqlClient(dbConnInfo, 1);

	if (!dbClient) {
		std::cerr << "Cannot initialize database connection" << std::endl;
		return 1;
	}

	auto transaction = dbClient->newTransaction();
	drogon::orm::Mapper<News> mapper(transaction);

	for (auto n : news) {
		try {
			mapper.insert(n);
			std::cout << "Added news with ID " << n.getId() << std::endl;
		} catch (const drogon::orm::DrogonDbException& e) {
			std::cerr << "Failed to add news: " << e.base().what() << std::endl;
			transaction->rollback();
			return false;
		}
	}
	return true;
}
