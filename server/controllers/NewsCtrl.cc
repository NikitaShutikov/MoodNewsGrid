#include "NewsCtrl.h"

#include <drogon/HttpController.h>
#include <drogon/orm/RestfulController.h>

using namespace drogon;
using namespace drogon::orm;
using namespace drogon_model::mydb;

inline drogon::HttpResponsePtr newHttpJsonResponseUtf8(
	const Json::Value& data) {
	Json::StreamWriterBuilder builder;
	builder["emitUTF8"] = true;
	std::string body = Json::writeString(builder, data);

	auto resp = drogon::HttpResponse::newHttpResponse();
	resp->setStatusCode(drogon::k200OK);
	resp->setContentTypeCode(drogon::CT_APPLICATION_JSON);
	resp->setBody(body);
	return resp;
}

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
				std::string mood = iter->second;
				std::string originalText = r.getValueOfOriginalText();

				std::string prompt =
					"Ты — редактор новостей. Переписывай тексты в заданном "
					"тоне, "
					"сохраняя все факты, даты, имена и цифры. Отвечай только "
					"переписанным текстом, без пояснений. "
					"Перепиши следующий текст новости в " +
					mood + " тоне. ";

				if (mood == "happy") {
					prompt +=
						"Сделай текст жизнерадостным, добавь позитивные "
						"эпитеты, используй восклицания. ";
				} else if (mood == "sad") {
					prompt +=
						"Сделай текст печальным, трагичным и грустным, "
						"используй "
						"грустные обороты, "
						"подчеркни потери и разочарования. ";
				} else if (mood == "ironic") {
					prompt +=
						"Добавь сарказм и иронию, используй преувеличения, "
						"высмеивай абсурдность ситуации. ";
				} else if (mood == "neutral") {
					prompt +=
						"Перескажи текст сухо и фактически, без эмоций, как "
						"официальная сводка. ";
				} else {
					Json::Value error;
					error["error"] =
						"Invalid mood parameter. Allowed: happy, sad, neutral, "
						"ironic. ";
					auto resp =
						drogon::HttpResponse::newHttpJsonResponse(error);
					resp->setStatusCode(drogon::k400BadRequest);
					(*callbackPtr)(resp);
					return;
				}
				prompt +=
					"Сохрани все факты, даты, имена, цифры и ссылки. Ответь "
					"только "
					"переписанным текстом. Текст новости:\n" +
					originalText;

				Json::Value requestBody;
				requestBody["model"] = "gpt-4";
				Json::Value messages(Json::arrayValue);

				Json::Value userMsg;
				userMsg["role"] = "user";
				userMsg["content"] = prompt;
				messages.append(userMsg);

				requestBody["messages"] = messages;
				requestBody["temperature"] = 0.95;

				auto client =
					drogon::HttpClient::newHttpClient("http://llama:8080");
				auto request = drogon::HttpRequest::newHttpRequest();
				request->setMethod(drogon::Post);
				request->setPath("/v1/chat/completions");
				request->setContentTypeCode(drogon::CT_APPLICATION_JSON);
				request->setBody(requestBody.toStyledString());

				client->sendRequest(request, [callbackPtr, r](
												 drogon::ReqResult result,
												 const drogon::HttpResponsePtr&
													 response) {
					Json::Value ret = r.toJson();

					if (result == drogon::ReqResult::Ok && response &&
						response->getStatusCode() == 200) {
						auto respJson = response->getJsonObject();
						if (respJson) {
							try {
								std::string rewritten =
									(*respJson)["choices"][0]["message"]
											   ["content"]
												   .asString();
								ret["text_with_mood"] = rewritten;
							} catch (const std::exception& e) {
								std::cerr << "Failed to extract text from "
											 "llama response: "
										  << e.what() << std::endl;
							}
						} else {
							std::cerr
								<< "Failed to parse JSON from llama response"
								<< std::endl;
						}
					} else {
						std::cerr << "Llama API request failed (result="
								  << static_cast<int>(result) << ")"
								  << std::endl;
					}

					auto resp = newHttpJsonResponseUtf8(ret);
					(*callbackPtr)(resp);
				});
			} else {
				(*callbackPtr)(newHttpJsonResponseUtf8(r.toJson()));
			}
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
