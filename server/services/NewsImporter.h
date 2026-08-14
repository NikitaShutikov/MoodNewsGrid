#include <News.h>
#include <drogon/orm/DbConfig.h>

#include <vector>

class NewsImporter {
   public:
	/**
	 * @param dbConfig Конфигурация базы данных
	 * @param newsUrl URL новостей в RSS формате
	 */
	NewsImporter(drogon::orm::MysqlConfig dbConfig, std::string newsUrl);

	/**
	 * @brief Получает новости
	 */
	std::vector<drogon_model::mydb::News> fetchNews();

	/**
	 * @brief Заполняет базу данных новостями
	 *
	 * @param news список новостей
	 * @return true в случае успешного заполнения, и false в случае ошибки
	 */
	bool seedNews(std::vector<drogon_model::mydb::News> news);

   private:
	std::string dbConnInfo;
	std::string newsUrl;
};
