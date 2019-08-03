#pragma once
#include "Base/Base.h"
using namespace Public::Base;

namespace Public {
namespace HTTP {

struct ContentInfo
{
	const char* contentType;
	const char* filetype;
};

struct MediaType
{
	static ContentInfo* info(uint32_t& len)
	{
		static ContentInfo mimetypes[] =
		{
			{"text/html",						"html"},
			{"text/html",						"htm"},
			{"text/html",						"shtml"},
			{"text/css",						"css"},
			{"text/xml",						"xml"},
			{"image/gif",						"gif"},
			{"image/jpeg",						"jpeg"},
			{"image/jpeg",						"jpg"},
			{"application/x-javascript",		"js"},
			{"application/atom+xml",			"atom"},
			{"application/rss+xml",				"rss"},

			{"text/mathml",						"mml"},
			{"text/plain",						"txt"},
			{"text/vnd.sun.j2me.app-descriptor","jad"},
			{"text/vnd.wap.wml",				"wml"},
			{"text/x-component",				"htc"},

			{"image/png",						"png"},
			{"image/tiff",						"tif"},
			{"image/tiff",						"tiff"},
			{"image/vnd.wap.wbmp",				"wbmp"},
			{"image/x-icon",					"ico"},
			{"image/x-jng",						"jng"},
			{"image/x-ms-bmp",					"bmp"},
			{"image/svg+xml",					"svg"},
			{"image/svg+xml",					"svgz"},
			{"image/webp",						"webp"},

			{"application/java-archive",		"jar"},
			{"application/java-archive ",		"war"},
			{"application/java-archive",		"ear"},
			{"application/mac-binhex40",		"hqx"},
			{"application/msword",				"doc"},
			{"application/pdf",					"pdf"},
			{"application/postscript",			"ps"},
			{"application/postscript",			"eps"},
			{"application/postscript",			"ai"},
			{"application/rtf",					"rtf"},
			{"application/vnd.ms-excel",		"xls"},
			{"application/vnd.ms-powerpoint",	"ppt"},
			{"application/vnd.wap.wmlc",		"wmlc"},
			{"application/vnd.google-earth.kml+xml","kml"},
			{"application/vnd.google-earth.kmz","kmz"},
			{"application/x-7z-compressed",		"7z"},
			{"application/x-cocoa",				"cco"},
			{"application/x-java-archive-diff",	"jardiff"},
			{"application/x-java-jnlp-file",	"jnlp"},
			{"application/x-makeself",			"run"},
			{"application/x-perl",				"pl"},
			{"application/x-perl",				"pm"},
			{"application/x-pilot",				"prc"},
			{"application/x-pilot",				"pdb"},
			{"application/x-rar-compressed",	"rar"},
			{"application/x-redhat-package-manager","rpm"},
			{"application/x-sea",				"sea"},
			{"application/x-shockwave-flash",	"swf"},
			{"application/x-stuffit",			"sit"},
			{"application/x-tcl",				"tcl"},
			{"application/x-tcl",				"tk"},
			{"application/x-x509-ca-cert",		"der"},
			{"application/x-x509-ca-cert",		"pem"},
			{"application/x-x509-ca-cert",		"crt"},
			{"application/x-xpinstall",			"xpi"},
			{"application/xhtml+xml",			"xhtml"},
			{"application/zip",					"zip"},

			{"application/octet-stream",		"bin"},
			{"application/octet-stream",		"exe"},
			{"application/octet-stream",		"dll"},
			{"application/octet-stream",		"deb"},
			{"application/octet-stream",		"dmg"},
			{"application/octet-stream",		"eot"},
			{"application/octet-stream",		"iso"},
			{"application/octet-stream",		"img"},
			{"application/octet-stream",		"msi"},
			{"application/octet-stream",		"msp"},
			{"application/octet-stream",		"msm"},

			{"audio/midi",						"mid"},
			{"audio/midi",						"midi"},
			{"audio/midi",						"kar"},
			{"audio/mpeg",						"mp3"},
			{"audio/ogg ",						"ogg"},
			{"audio/x-m4",						"m4a"},
			{"audio/x-realaudio",				"ra"},

			{"video/3gpp",						"3gpp"},
			{"video/3gpp",						"3gp"},
			{"video/mp4",						"mp4"},
			{"video/mpeg",						"mpeg"},
			{"video/mpeg",						"mpg"},
			{"video/quicktime",					"mov"},
			{"video/webm",						"webm"},
			{"video/x-flv",						"flv"},
			{"video/x-m4v",						"m4v"},
			{"video/x-mng",						"mng"},
			{"video/x-ms-asf",					"asx"},
			{"video/x-ms-asf",					"asf"},
			{"video/x-ms-wmv",					"wmv" },
			{"video/x-msvideo",					"avi" },
			{"video/mp2t",						"ts" },
			{"application/vnd.apple.mpegurl",	"m3u8" },
		};

		len = sizeof(mimetypes) / sizeof(ContentInfo);

		return mimetypes;
	}
};

}
}

