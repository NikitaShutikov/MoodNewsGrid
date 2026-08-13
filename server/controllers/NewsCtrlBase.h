#pragma once

#include <drogon/HttpController.h>
#include <drogon/orm/RestfulController.h>

#include "News.h"
using namespace drogon;
using namespace drogon::orm;
using namespace drogon_model::mydb;

class NewsCtrlBase : public RestfulController {
   public:
	void getOne(const HttpRequestPtr& req,
				std::function<void(const HttpResponsePtr&)>&& callback,
				News::PrimaryKeyType&& id);
	void get(const HttpRequestPtr& req,
			 std::function<void(const HttpResponsePtr&)>&& callback);

	orm::DbClientPtr getDbClient() {
		return drogon::app().getDbClient(dbClientName_);
	}

   protected:
	NewsCtrlBase();
	const std::string dbClientName_{"default"};
};
