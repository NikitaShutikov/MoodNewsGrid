#pragma once

#include <drogon/HttpController.h>
#include <drogon/orm/RestfulController.h>

#include "News.h"
using namespace drogon;
using namespace drogon_model::mydb;

class NewsCtrl : public drogon::HttpController<NewsCtrl> {
   public:
	METHOD_LIST_BEGIN
	ADD_METHOD_TO(NewsCtrl::getOne, "/news/{1}", Get, Options);
	ADD_METHOD_TO(NewsCtrl::get, "/news", Get, Options);
	METHOD_LIST_END

	void getOne(const HttpRequestPtr& req,
				std::function<void(const HttpResponsePtr&)>&& callback,
				News::PrimaryKeyType&& id);
	void get(const HttpRequestPtr& req,
			 std::function<void(const HttpResponsePtr&)>&& callback);

	orm::DbClientPtr getDbClient() {
		return drogon::app().getDbClient("default");
	}
};
