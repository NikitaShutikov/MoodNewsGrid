#include <News.h>
#include <drogon/orm/DbConfig.h>

#include <vector>

class NewsImporter {
   public:
	NewsImporter(drogon::orm::MysqlConfig dbConfig);

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
};
