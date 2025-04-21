#ifndef RESPONSE_HPP
# define RESPONSE_HPP
# include "Request.hpp"
# include <iostream>
# include "CgiHandler.hpp"
# include "Logger.hpp"
# include "Utils.hpp"

#include <cerrno>
#include <cstring> // For strerror

enum FileType {
    NotFound,
    NormalFile,
    Directory
};

enum ResponseState {
    READY,
    PROCESSING_CGI,
    COMPLETE
};

class Response {
	private:
		Request		*_request;
		STR			_body;
		HttpConfig	*_config;
		std::map<STR, STR>			_all_mime_types; // _all_mime_types["extention"] = "name", _all_mime_types[".html"] = "text/html"
															// the order is not in reverse as several extentions can have same name

		std::map<int, STR>			_all_status_codes;
		STR							handleGET(STR best_path, bool isDIR);
		STR							handlePOST(STR path);
		STR							handleDELETE(STR path);
		STR							getMime(STR path);
		STR							handleDIR(STR path);
		void						selectIndexIndexes(VECTOR<STR> indexes, STR &best_match, float &match_quality, STR dir_path);
		STR							selectIndexAll(LocationConfig* location, STR dir_path);
		FileType 					checkFile(const STR& path);
		LocationConfig 				*buildDirPath(ServerConfig *matchServer, STR &full_path, bool &isDIR);
		int							buildIndexPath(LocationConfig *matchLocation, STR &best_file_path, STR dir_path);
		STR							matchMethod(STR path, bool isDIR, LocationConfig *matchLocation);
		STR							checkRedirect(LocationConfig *matchLocation);
		bool						checkBodySize(LocationConfig *matchLocation);
        
        // CGI processing
        CgiHandler*                 _cgi_handler;
        ResponseState               _state;
        STR                         _response_buffer;

	public:
		Response();
		Response(Request *request, HttpConfig *config);
		Response(const Response &obj);
		~Response();

		void	setRequest(Request *request);
		void	setConfig(HttpConfig *config);
		STR		createResponse(int statusCode, const STR& contentType, const STR& body, const STR& extra);
		STR		createErrorResponse(int statusCode, const STR& contentType, const STR& body, AConfigBase *base);
		STR		getResponse();
		void	clear();
        
        // CGI integration methods
        bool    isResponseReady() const { return _state != PROCESSING_CGI; }
        int     getCgiOutputFd() const; // Returns -1 if no CGI is processing
        bool    processCgiOutput(); // Process CGI output data, returns true if completed
        STR     getFinalResponse(); // Get the final response after CGI completes
};

#endif
