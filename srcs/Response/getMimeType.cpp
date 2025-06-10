#include "Response.hpp"

void	Response::init_mimetypes(MAP<STR, STR>	&mime_types) {
	mime_types[".html"] = "text/html";
	mime_types[".htm"] = "text/html";
	mime_types[".shtml"] = "text/html";
	mime_types[".css"] = "text/css";
	mime_types[".gif"] = "image/gif";
	mime_types[".jpeg"] = "image/jpeg";
	mime_types[".jpg"] = "image/jpeg";
	mime_types[".js"] = "application/javascript";
	mime_types[".atom"] = "application/atom+xml";
	mime_types[".rss"] = "application/rss+xml";
	mime_types[".mml"] = "text/mathml";
	mime_types[".txt"] = "text/plain";
	mime_types[".jad"] = "text/vnd.sun.j2me.app-descriptor";
	mime_types[".wml"] = "text/vnd.wap.wml";
	mime_types[".htc"] = "text/x-component";
	mime_types[".avif"] = "image/avif";
	mime_types[".png"] = "image/png";
	mime_types[".svg"] = "image/svg+xml";
	mime_types[".svgz"] = "image/svg+xml";
	mime_types[".tif"] = "image/tiff";
	mime_types[".tiff"] = "image/tiff";
	mime_types[".wbmp"] = "image/vnd.wap.wbmp";
	mime_types[".webp"] = "image/webp";
	mime_types[".ico"] = "image/x-icon";
	mime_types[".jng"] = "image/x-jng";
	mime_types[".bmp"] = "image/x-ms-bmp";
	mime_types[".woff"] = "font/woff";
	mime_types[".woff2"] = "font/woff2";
	mime_types[".jar"] = "application/java-archive";
	mime_types[".war"] = "application/java-archive";
	mime_types[".ear"] = "application/java-archive";
	mime_types[".json"] = "application/json";
	mime_types[".hqx"] = "application/mac-binhex40";
	mime_types[".doc"] = "application/msword";
	mime_types[".pdf"] = "application/pdf";
	mime_types[".ps"] = "application/postscript";
	mime_types[".eps"] = "application/postscript";
	mime_types[".ai"] = "application/postscript";
	mime_types[".rtf"] = "application/rtf";
	mime_types[".m3u8"] = "application/vnd.apple.mpegurl";
	mime_types[".kml"] = "application/vnd.google-earth.kml+xml";
	mime_types[".kmz"] = "application/vnd.google-earth.kmz";
	mime_types[".xls"] = "application/vnd.ms-excel";
	mime_types[".eot"] = "application/vnd.ms-fontobject";
	mime_types[".ppt"] = "application/vnd.ms-powerpoint";
	mime_types[".odg"] = "application/vnd.oasis.opendocument.graphics";
	mime_types[".odp"] = "application/vnd.oasis.opendocument.presentation";
	mime_types[".ods"] = "application/vnd.oasis.opendocument.spreadsheet";
	mime_types[".odt"] = "application/vnd.oasis.opendocument.text";
	mime_types[".pptx"] = "application/vnd.openxmlformats-officedocument.presentationml.presentation";
	mime_types[".xlsx"] = "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
	mime_types[".docx"] = "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
	mime_types[".wmlc"] = "application/vnd.wap.wmlc";
	mime_types[".wasm"] = "application/wasm";
	mime_types[".7z"] = "application/x-7z-compressed";
	mime_types[".cco"] = "application/x-cocoa";
	mime_types[".jardiff"] = "application/x-java-archive-diff";
	mime_types[".jnlp"] = "application/x-java-jnlp-file";
	mime_types[".run"] = "application/x-makeself";
	mime_types[".pl"] = "application/x-perl";
	mime_types[".pm"] = "application/x-perl";
	mime_types[".prc"] = "application/x-pilot";
	mime_types[".pdb"] = "application/x-pilot";
	mime_types[".rar"] = "application/x-rar-compressed";
	mime_types[".rpm"] = "application/x-redhat-package-manager";
	mime_types[".sea"] = "application/x-sea";
	mime_types[".swf"] = "application/x-shockwave-flash";
	mime_types[".sit"] = "application/x-stuffit";
	mime_types[".tcl"] = "application/x-tcl";
	mime_types[".tk"] = "application/x-tcl";
	mime_types[".der"] = "application/x-x509-ca-cert";
	mime_types[".pem"] = "application/x-x509-ca-cert";
	mime_types[".crt"] = "application/x-x509-ca-cert";
	mime_types[".xpi"] = "application/x-xpinstall";
	mime_types[".xhtml"] = "application/xhtml+xml";
	mime_types[".xspf"] = "application/xspf+xml";
	mime_types[".zip"] = "application/zip";
	mime_types[".bin"] = "application/octet-stream";
	mime_types[".exe"] = "application/octet-stream";
	mime_types[".dll"] = "application/octet-stream";
	mime_types[".deb"] = "application/octet-stream";
	mime_types[".dmg"] = "application/octet-stream";
	mime_types[".iso"] = "application/octet-stream";
	mime_types[".img"] = "application/octet-stream";
	mime_types[".msi"] = "application/octet-stream";
	mime_types[".msp"] = "application/octet-stream";
	mime_types[".msm"] = "application/octet-stream";
	mime_types[".mid"] = "audio/midi";
	mime_types[".midi"] = "audio/midi";
	mime_types[".kar"] = "audio/midi";
	mime_types[".mp3"] = "audio/mpeg";
	mime_types[".ogg"] = "audio/ogg";
	mime_types[".m4a"] = "audio/x-m4a";
	mime_types[".ra"] = "audio/x-realaudio";
	mime_types[".3gpp"] = "video/3gpp";
	mime_types[".3gp"] = "video/3gpp";
	mime_types[".ts"] = "video/mp2t";
	mime_types[".mp4"] = "video/mp4";
	mime_types[".mpeg"] = "video/mpeg";
	mime_types[".mpg"] = "video/mpeg";
	mime_types[".mov"] = "video/quicktime";
	mime_types[".webm"] = "video/webm";
	mime_types[".flv"] = "video/x-flv";
	mime_types[".m4v"] = "video/x-m4v";
	mime_types[".mng"] = "video/x-mng";
	mime_types[".asx"] = "video/x-ms-asf";
	mime_types[".asf"] = "video/x-ms-asf";
	mime_types[".wmv"] = "video/x-ms-wmv";
	mime_types[".avi"] = "video/x-msvideo";
}

void	Response::init_status_codes(MAP<int, STR>	&status_codes) {
	// Informational responses (100-199)
	status_codes[100] = "100 Continue";
	status_codes[101] = "101 Switching Protocols";
	status_codes[102] = "102 Processing";
	status_codes[103] = "103 Early Hints";

	// Successful responses (200-299)
	status_codes[200] = "200 OK";
	status_codes[201] = "201 Created";
	status_codes[202] = "202 Accepted";
	status_codes[203] = "203 Non-Authoritative Information";
	status_codes[204] = "204 No Content";
	status_codes[205] = "205 Reset Content";
	status_codes[206] = "206 Partial Content";
	status_codes[207] = "207 Multi-Status";
	status_codes[208] = "208 Already Reported";
	status_codes[226] = "226 IM Used";

	// Redirection messages (300-399)
	status_codes[300] = "300 Multiple Choices";
	status_codes[301] = "301 Moved Permanently";
	status_codes[302] = "302 Found";
	status_codes[303] = "303 See Other";
	status_codes[304] = "304 Not Modified";
	status_codes[305] = "305 Use Proxy";
	status_codes[307] = "307 Temporary Redirect";
	status_codes[308] = "308 Permanent Redirect";

	// Client error responses (400-499)
	status_codes[400] = "400 Bad Request";
	status_codes[401] = "401 Unauthorized";
	status_codes[402] = "402 Payment Required";
	status_codes[403] = "403 Forbidden";
	status_codes[404] = "404 Not Found";
	status_codes[405] = "405 Method Not Allowed";
	status_codes[406] = "406 Not Acceptable";
	status_codes[407] = "407 Proxy Authentication Required";
	status_codes[408] = "408 Request Timeout";
	status_codes[409] = "409 Conflict";
	status_codes[410] = "410 Gone";
	status_codes[411] = "411 Length Required";
	status_codes[412] = "412 Precondition Failed";
	status_codes[413] = "413 Payload Too Large";
	status_codes[414] = "414 URI Too Long";
	status_codes[415] = "415 Unsupported Media Type";
	status_codes[416] = "416 Range Not Satisfiable";
	status_codes[417] = "417 Expectation Failed";
	status_codes[418] = "418 I'm a teapot";
	status_codes[421] = "421 Misdirected Request";
	status_codes[422] = "422 Unprocessable Entity";
	status_codes[423] = "423 Locked";
	status_codes[424] = "424 Failed Dependency";
	status_codes[425] = "425 Too Early";
	status_codes[426] = "426 Upgrade Required";
	status_codes[428] = "428 Precondition Required";
	status_codes[429] = "429 Too Many Requests";
	status_codes[431] = "431 Request Header Fields Too Large";
	status_codes[451] = "451 Unavailable For Legal Reasons";

	// Server error responses (500-599)
	status_codes[500] = "500 Internal Server Error";
	status_codes[501] = "501 Not Implemented";
	status_codes[502] = "502 Bad Gateway";
	status_codes[503] = "503 Service Unavailable";
	status_codes[504] = "504 Gateway Timeout";
	status_codes[505] = "505 HTTP Version Not Supported";
	status_codes[506] = "506 Variant Also Negotiates";
	status_codes[507] = "507 Insufficient Storage";
	status_codes[508] = "508 Loop Detected";
	status_codes[510] = "510 Not Extended";
	status_codes[511] = "511 Network Authentication Required";
}

STR Response::getMimeType(const STR& path) {
    if (ends_with(path, ".html") || ends_with(path, ".htm")) return "text/html; charset=utf-8";
    if (ends_with(path, ".css")) return "text/css; charset=utf-8";
    if (ends_with(path, ".js")) return "text/javascript; charset=utf-8";
    if (ends_with(path, ".json")) return "application/json; charset=utf-8";
    if (ends_with(path, ".jpg") || ends_with(path, ".jpeg")) return "image/jpeg";
    if (ends_with(path, ".png")) return "image/png";
    if (ends_with(path, ".gif")) return "image/gif";
    if (ends_with(path, ".svg")) return "image/svg+xml";
    if (ends_with(path, ".ico")) return "image/x-icon";
    if (ends_with(path, ".txt")) return "text/plain; charset=utf-8";
    if (ends_with(path, ".pdf")) return "application/pdf";
    return "text/plain; charset=utf-8";  // default
}

STR	Response::getMime(STR path) {

	// std::cerr << "ending mime : " << ending << ", for path " << path << "\n";
	try
	{
		STR	ending = path.substr(path.find_last_of('.'));
		return _all_mime_types.at(ending);
	}
	catch(const std::exception& e)
	{
		return "text/plain";
	}

	return "text/plain";
}
