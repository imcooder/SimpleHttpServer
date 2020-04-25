//
// mime_types.cpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "stdafx.h"
#include "mime_types.hpp"
namespace http 
{
	namespace server3
	{
		namespace mime_types
		{

			struct mapping
			{
				const char* extension;
				const char* mime_type;
			} mappings[] =
			{
				{ "txt", "text/plain" },				
				{ "htm", "text/html" },
				{ "html", "text/html" },
				{ "php", "text/html" },
				{ "jpg", "image/jpeg" },
				{ "png", "image/png" },
				{ "gif", "image/gif" },
				{ "icon", "image/x-icon" },
				{ "ico", "image/x-icon" },
				{ "css", "text/css" },
				{ "xml", "text/xml" },
				{ "js", "application/x-javascript" },
				{ "json", "text/json" },				
				{"default","text/plain"},
				{ 0, 0 } // Marks end of list.
			};

			std::string extension_to_type(const std::string& extension)
			{
				for (mapping* m = mappings; m && m->extension; ++m)
				{
					if (m->extension == extension)
					{
						return m->mime_type;
					}
				}
				return extension;
			}

		} // namespace mime_types
	} // namespace server3
} // namespace http
