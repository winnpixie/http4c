#include "utilities.h"
#include <string.h>

char *get_content_type(const char *path)
{
	char *ext = strrchr(path, '.');
	if (ext == NULL)
	{
		return "application/octet-stream";
	}

	if (strcmp(ext, ".css") == 0)
	{
		return "text/css";
	} else if (strcmp(ext, ".html") == 0)
	{
		return "text/html";
	} else if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0)
	{
		return "image/jpeg";
	} else if (strcmp(ext, ".js") == 0)
	{
		return "text/javascript";
	} else if (strcmp(ext, ".png") == 0)
	{
		return "image/png";
	}

	return "application/octet-stream";
}
