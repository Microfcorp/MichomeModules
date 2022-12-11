#ifndef FSBrowser_h
#define FSBrowser_h
static const char TEXT_PLAIN[] PROGMEM = "text/plain";
static const char FS_INIT_ERROR[] PROGMEM = "FS INIT ERROR";
static const char FILE_NOT_FOUND[] PROGMEM = "FileNotFound";

bool fsOK = true;
ESP8266WebServer *server2;

#if defined USE_SPIFFS
	#include <FS.h>
	const char* fsName = "SPIFFS";
	FS* fileSystem = &SPIFFS;
	SPIFFSConfig fileSystemConfig = SPIFFSConfig();
#elif defined USE_LITTLEFS
	#include <LittleFS.h>
	const char* fsName = "LittleFS";
	FS* fileSystem = &LittleFS;
	LittleFSConfig fileSystemConfig = LittleFSConfig();
#elif defined USE_SDFS
	#include <SDFS.h>
	const char* fsName = "SDFS";
	FS* fileSystem = &SDFS;
	SDFSConfig fileSystemConfig = SDFSConfig();
	// fileSystemConfig.setCSPin(chipSelectPin);
#else
#error Please select a filesystem first by uncommenting one of the "#define USE_xxx" lines at the beginning of the sketch.
#endif
String unsupportedFiles = String();

File uploadFile;

////////////////////////////////
// Utils to return HTTP codes, and determine content-type

void replyOK() {
  (*server2).send(200, FPSTR(TEXT_PLAIN), "");
}

void replyOKWithMsg(String msg) {
  (*server2).send(200, FPSTR(TEXT_PLAIN), msg);
}

void replyNotFound(String msg) {
  (*server2).send(404, FPSTR(TEXT_PLAIN), msg);
}

void replyBadRequest(String msg) {
  (*server2).send(400, FPSTR(TEXT_PLAIN), msg + "\r\n");
}

void replyServerError(String msg) {
  (*server2).send(500, FPSTR(TEXT_PLAIN), msg + "\r\n");
}

#ifdef USE_SPIFFS
/*
   Checks filename for character combinations that are not supported by FSBrowser (alhtough valid on SPIFFS).
   Returns an empty String if supported, or detail of error(s) if unsupported
*/
String checkForUnsupportedPath(String filename) {
  String error = String();
  if (!filename.startsWith("/")) {
	error += F("!NO_LEADING_SLASH! ");
  }
  if (filename.indexOf("//") != -1) {
	error += F("!DOUBLE_SLASH! ");
  }
  if (filename.endsWith("/")) {
	error += F("!TRAILING_SLASH! ");
  }
  return error;
}
#endif

/*
   Return the FS type, status and size info
*/
void handleStatus() {
  FSInfo fs_info;
  String json;
  json.reserve(128);

  json = "{\"type\":\"";
  json += fsName;
  json += "\", \"isOk\":";
  if (fsOK) {
	fileSystem->info(fs_info);
	json += F("\"true\", \"totalBytes\":\"");
	json += fs_info.totalBytes;
	json += F("\", \"usedBytes\":\"");
	json += fs_info.usedBytes;
	json += "\"";
  } else {
	json += "\"false\"";
  }
  json += F(",\"unsupportedFiles\":\"");
  json += unsupportedFiles;
  json += "\"}";

  (*server2).send(200, "application/json", json);
}


/*
   Return the list of files in the directory specified by the "dir" query string parameter.
   Also demonstrates the use of chuncked responses.
*/
void handleFileList() {
  if (!fsOK) {
	return replyServerError(FPSTR(FS_INIT_ERROR));
  }

  String path = "";
  if (!(*server2).hasArg("dir"))
	path = "/";
  else	  
	path = (*server2).arg("dir");

  if (path != "/" && !fileSystem->exists(path)) {
	return replyBadRequest("BAD PATH");
  }

  Dir dir = fileSystem->openDir(path);
  path.clear();

  // use HTTP/1.1 Chunked response to avoid building a huge temporary string
  if (!(*server2).chunkedResponseModeStart(200, "text/json")) {
	(*server2).send(505, F("text/html"), F("HTTP1.1 required"));
	return;
  }

  // use the same string for every line
  String output;
  output.reserve(64);
  while (dir.next()) {
#ifdef USE_SPIFFS
	String error = checkForUnsupportedPath(dir.fileName());
	if (error.length() > 0) {
	  continue;
	}
#endif
	if (output.length()) {
	  // send string from previous iteration
	  // as an HTTP chunk
	  (*server2).sendContent(output);
	  output = ',';
	} else {
	  output = '[';
	}

	output += "{\"type\":\"";
	if (dir.isDirectory()) {
	  output += "dir";
	} else {
	  output += F("file\",\"size\":\"");
	  output += dir.fileSize();
	}

	output += F("\",\"name\":\"");
	// Always return names without leading "/"
	if (dir.fileName()[0] == '/') {
	  output += &(dir.fileName()[1]);
	} else {
	  output += dir.fileName();
	}

	output += "\"}";
  }

  // send last string
  output += "]";
  (*server2).sendContent(output);
  (*server2).chunkedResponseFinalize();
}


/*
   Read the given file from the filesystem and stream it back to the client
*/
bool handleFileRead(String path) {
  if (!fsOK) {
	replyServerError(FPSTR(FS_INIT_ERROR));
	return true;
  }

  if (path.endsWith("/")) {
	path = path.substring(0, path.length()-1); //lastExistingParent
  }

  String contentType;
  if ((*server2).hasArg("download")) {
	contentType = F("application/octet-stream");
  } else {
	contentType = mime::getContentType(path); //Баг века. Ошибка в файлах ядра, где идет парсинг мим типов
	if(contentType == ".txt"){
		contentType = F("text/plain");
	}
  }

  if (!fileSystem->exists(path)) {
	// File not found, try gzip version
	path = path + ".gz";
  }
  if (fileSystem->exists(path)) {
	File file = fileSystem->open(path, "r");
	if ((*server2).streamFile(file, contentType) != file.size()) {
	}
	file.close();
	return true;
  }

  return false;
}


/*
   As some FS (e.g. LittleFS) delete the parent folder when the last child has been removed,
   return the path of the closest parent still existing
*/
String lastExistingParent(String path) {
  while (!path.isEmpty() && !fileSystem->exists(path)) {
	if (path.lastIndexOf('/') > 0) {
	  path = path.substring(0, path.lastIndexOf('/'));
	} else {
	  path = String();  // No slash => the top folder does not exist
	}
  }
  return path;
}

/*
   Handle the creation/rename of a new file
   Operation      | req.responseText
   ---------------+--------------------------------------------------------------
   Create file    | parent of created file
   Create folder  | parent of created folder
   Rename file    | parent of source file
   Move file      | parent of source file, or remaining ancestor
   Rename folder  | parent of source folder
   Move folder    | parent of source folder, or remaining ancestor
*/
void handleFileCreate() {
  if (!fsOK) {
	return replyServerError(FPSTR(FS_INIT_ERROR));
  }

  String path = (*server2).arg("path");
  if (path.isEmpty()) {
	return replyBadRequest(F("PATH ARG MISSING"));
  }

#ifdef USE_SPIFFS
  if (checkForUnsupportedPath(path).length() > 0) {
	return replyServerError(F("INVALID FILENAME"));
  }
#endif

  if (path == "/") {
	return replyBadRequest("BAD PATH");
  }
  if (fileSystem->exists(path)) {
	return replyBadRequest(F("PATH FILE EXISTS"));
  }

  String src = (*server2).arg("src");
  if (src.isEmpty()) {
	// No source specified: creation
	if (path.endsWith("/")) {
	  // Create a folder
	  path.remove(path.length() - 1);
	  if (!fileSystem->mkdir(path)) {
		return replyServerError(F("MKDIR FAILED"));
	  }
	} else {
	  // Create a file
	  File file = fileSystem->open(path, "w");
	  if (file) {
		file.write((const char *)0);
		file.close();
	  } else {
		return replyServerError(F("CREATE FAILED"));
	  }
	}
	if (path.lastIndexOf('/') > -1) {
	  path = path.substring(0, path.lastIndexOf('/'));
	}
	replyOKWithMsg(path);
  } else {
	// Source specified: rename
	if (src == "/") {
	  return replyBadRequest("BAD SRC");
	}
	if (!fileSystem->exists(src)) {
	  return replyBadRequest(F("SRC FILE NOT FOUND"));
	}


	if (path.endsWith("/")) {
	  path.remove(path.length() - 1);
	}
	if (src.endsWith("/")) {
	  src.remove(src.length() - 1);
	}
	if (!fileSystem->rename(src, path)) {
	  return replyServerError(F("RENAME FAILED"));
	}
	replyOKWithMsg(lastExistingParent(src));
  }
}


/*
   Delete the file or folder designed by the given path.
   If it's a file, delete it.
   If it's a folder, delete all nested contents first then the folder itself

   IMPORTANT NOTE: using recursion is generally not recommended on embedded devices and can lead to crashes (stack overflow errors).
   This use is just for demonstration purpose, and FSBrowser might crash in case of deeply nested filesystems.
   Please don't do this on a production system.
*/
void deleteRecursive(String path) {
  File file = fileSystem->open(path, "r");
  bool isDir = file.isDirectory();
  file.close();

  // If it's a plain file, delete it
  if (!isDir) {
	fileSystem->remove(path);
	return;
  }

  // Otherwise delete its contents first
  Dir dir = fileSystem->openDir(path);

  while (dir.next()) {
	deleteRecursive(path + '/' + dir.fileName());
  }

  // Then delete the folder itself
  fileSystem->rmdir(path);
}


/*
   Handle a file deletion request
   Operation      | req.responseText
   ---------------+--------------------------------------------------------------
   Delete file    | parent of deleted file, or remaining ancestor
   Delete folder  | parent of deleted folder, or remaining ancestor
*/
void handleFileDelete() {
  if (!fsOK) {
	return replyServerError(FPSTR(FS_INIT_ERROR));
  }

  String path = (*server2).arg(0);
  if (path.isEmpty() || path == "/") {
	return replyBadRequest("BAD PATH");
  }

  if (!fileSystem->exists(path)) {
	return replyNotFound(FPSTR(FILE_NOT_FOUND));
  }
  deleteRecursive(path);

  replyOKWithMsg(lastExistingParent(path));
}

/*
   Handle a file upload request
*/
void handleFileUpload() {
  if (!fsOK) {
	return replyServerError(FPSTR(FS_INIT_ERROR));
  }
  if ((*server2).uri() != "/edit") {
	return;
  }
  HTTPUpload& upload = (*server2).upload();
  if (upload.status == UPLOAD_FILE_START) {
	String filename = upload.filename;
	// Make sure paths always start with "/"
	if (!filename.startsWith("/")) {
	  filename = "/" + filename;
	}
	uploadFile = fileSystem->open(filename, "w");
	if (!uploadFile) {
	  return replyServerError(F("CREATE FAILED"));
	}
  } else if (upload.status == UPLOAD_FILE_WRITE) {
	if (uploadFile) {
	  size_t bytesWritten = uploadFile.write(upload.buf, upload.currentSize);
	  if (bytesWritten != upload.currentSize) {
		return replyServerError(F("WRITE FAILED"));
	  }
	}
  } else if (upload.status == UPLOAD_FILE_END) {
	if (uploadFile) {
	  uploadFile.close();
	}
  }
}


/*
   The "Not Found" handler catches all URI not explicitely declared in code
   First try to find and return the requested file from the filesystem,
   and if it fails, return a 404 page with debug information
*/
void handleNotFound() {
  if (!fsOK) {
	return replyServerError(FPSTR(FS_INIT_ERROR));
  }

  String uri = ESP8266WebServer::urlDecode((*server2).uri()); // required to read paths with blanks

  if (handleFileRead(uri)) {
	return;
  }

  // Dump debug data
  String message;
  message.reserve(100);
  message = F("Error: File not found\n\nURI: ");
  message += uri;
  message += F("\nMethod: ");
  message += ((*server2).method() == HTTP_GET) ? "GET" : "POST";
  message += F("\nArguments: ");
  message += (*server2).args();
  message += '\n';
  for (uint8_t i = 0; i < (*server2).args(); i++) {
	message += F(" NAME:");
	message += (*server2).argName(i);
	message += F("\n VALUE:");
	message += (*server2).arg(i);
	message += '\n';
  }
  message += "path=";
  message += (*server2).arg("path");
  message += '\n';

  return replyNotFound(message);
}

/*
   This specific handler returns the index.htm (or a gzipped version) from the /edit folder.
   If the file is not present but the flag INCLUDE_FALLBACK_INDEX_HTM has been set, falls back to the version
   embedded in the program code.
   Otherwise, fails with a 404 page with debug information
*/
void handleGetEdit() {
  if (!(*server2).hasArg("file")) {
	return replyBadRequest("BAD PATH");
  }

  String path = (*server2).arg("file");
  if (!fileSystem->exists(path)) {
	return replyBadRequest("BAD PATH");
  }
  
  if (handleFileRead(path)) {
	return;
  }
  
  return replyBadRequest("FILE READ ERROR");
}

void InitFSBrowser(ESP8266WebServer *server1){
	server2 = server1;
	
	////////////////////////////////
	// WEB SERVER INIT

	// Filesystem status
	(*server2).on("/fsstatus", HTTP_GET, handleStatus);

	// List directory
	(*server2).on("/fslist", HTTP_GET, handleFileList); //?dir=/  or /

	// Load editor
	(*server2).on("/fsedit", HTTP_GET, handleGetEdit); //?file=/f.txt  and ?download=1

	// Create file
	(*server2).on("/fsedit",  HTTP_PUT, handleFileCreate);

	// Delete file
	(*server2).on("/fsedit",  HTTP_DELETE, handleFileDelete);

	// Upload file
	// - first callback is called after the request has ended with all parsed arguments
	// - second callback handles file upload at that location
	(*server2).on("/fsedit",  HTTP_POST, replyOK, handleFileUpload);
}
#endif