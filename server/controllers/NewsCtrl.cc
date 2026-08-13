#include "NewsCtrl.h"

#include <drogon/HttpController.h>
#include <drogon/orm/RestfulController.h>

using namespace drogon;
using namespace drogon::orm;
using namespace drogon_model::mydb;

void NewsCtrl::getOne(const HttpRequestPtr& req,
					  std::function<void(const HttpResponsePtr&)>&& callback,
					  News::PrimaryKeyType&& id) {
	auto dbClientPtr = getDbClient();
	auto callbackPtr =
		std::make_shared<std::function<void(const HttpResponsePtr&)>>(
			std::move(callback));
	drogon::orm::Mapper<News> mapper(dbClientPtr);

	mapper.findByPrimaryKey(
		id,
		[req, callbackPtr, this](News r) {
			auto& parameters = req->parameters();
			auto iter = parameters.find("mood");
			if (iter != parameters.end()) {
				// TODO: добавить генерацию новости в зависимости от параметра
				// mood
			}
			(*callbackPtr)(HttpResponse::newHttpJsonResponse(r.toJson()));
		},
		[callbackPtr](const DrogonDbException& e) {
			const drogon::orm::UnexpectedRows* s =
				dynamic_cast<const drogon::orm::UnexpectedRows*>(&e.base());
			if (s) {
				auto resp = HttpResponse::newHttpResponse();
				resp->setStatusCode(k404NotFound);
				(*callbackPtr)(resp);
				return;
			}
			LOG_ERROR << e.base().what();
			Json::Value ret;
			ret["error"] = "database error";
			auto resp = HttpResponse::newHttpJsonResponse(ret);
			resp->setStatusCode(k500InternalServerError);
			(*callbackPtr)(resp);
		});
}

void NewsCtrl::get(const HttpRequestPtr& req,
				   std::function<void(const HttpResponsePtr&)>&& callback) {
	auto dbClientPtr = getDbClient();
	drogon::orm::Mapper<News> mapper(dbClientPtr);

	auto callbackPtr =
		std::make_shared<std::function<void(const HttpResponsePtr&)>>(
			std::move(callback));

	mapper.findAll(
		[req, callbackPtr, this](const std::vector<News>& v) {
			Json::Value ret;
			ret.resize(0);
			for (auto& obj : v) {
				ret.append(obj.toJson());
			}
			(*callbackPtr)(HttpResponse::newHttpJsonResponse(ret));
		},
		[callbackPtr](const DrogonDbException& e) {
			LOG_ERROR << e.base().what();
			Json::Value ret;
			ret["error"] = "database error";
			auto resp = HttpResponse::newHttpJsonResponse(ret);
			resp->setStatusCode(k500InternalServerError);
			(*callbackPtr)(resp);
		});
}
