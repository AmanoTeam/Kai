#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>

#include "fstream.h"
#include "filesystem.h"
#include "sslcerts.h"
#include "path.h"
#include "errors.h"

static const char* const SSL_CERTIFICATE_LOCATIONS[] = {
#if defined(__APPLE__)
	"/etc/ssl/cert.pem",
	"/System/Library/OpenSSL/certs/cert.pem"
#elif defined(__HAIKU__)
	"/system/data/ssl/CARootCertificates.pem"
#elif defined(__ANDROID__)
	"/data/data/com.termux/files/usr/etc/tls/cert.pem"
#elif defined(__linux__)
	"/etc/pki/tls/certs/ca-bundle.crt",
	"/etc/ssl/ca-bundle.pem",
	"/etc/ssl/certs/ca-certificates.crt",
	"/usr/share/ssl/certs/ca-bundle.crt"
#elif defined(__OpenBSD__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__DragonFly__)
	"/etc/ssl/cert.pem",
	"/etc/ssl/certs/ca-certificates.crt",
	"/usr/local/share/certs/ca-root-nss.crt"
#else
	"/etc/pki/ca-trust/extracted/pem/tls-ca-bundle.pem",
	"/etc/pki/tls/certs/ca-bundle.crt",
	"/etc/ssl/ca-bundle.pem",
	"/etc/ssl/cert.pem",
	"/etc/ssl/certs/ca-certificates.crt",
	"/usr/local/share/certs/ca-root-nss.crt",
	"/usr/share/ssl/certs/ca-bundle.crt"
#endif
};

static const char BUILTIN_CA_CERT_LOCATION[] = 
#if defined(_WIN32)
	"\\etc\\tls\\cert.pem";
#else
	"/etc/tls/cert.pem";
#endif

static struct curl_blob* cacerts = NULL;

static int sslcerts_load_file(const char* const name) {
	
	struct FStream* stream = NULL;
	char* buffer = NULL;
	
	long int file_size = 0;
	ssize_t rsize = 0;
	
	int status = 0;
	int err = M3U8ERR_SUCCESS;
	
	stream = fstream_open(name, FSTREAM_READ);
	
	if (stream == NULL) {
		err = M3U8ERR_FSTREAM_OPEN_FAILURE;
		goto end;
	}
	
	status = fstream_seek(stream, 0, FSTREAM_SEEK_END);
	
	if (status == -1) {
		err = M3U8ERR_FSTREAM_SEEK_FAILURE;
		goto end;
	}
	
	file_size = fstream_tell(stream);
	
	if (file_size == -1) {
		err = M3U8ERR_FSTREAM_TELL_FAILURE;
		goto end;
	}
	
	if (file_size == 0) {
		err = M3U8ERR_FSTREAM_READ_EMPTY_FILE;
		goto end;
	}
	
	status = fstream_seek(stream, 0, FSTREAM_SEEK_BEGIN);
	
	if (status == -1) {
		err = M3U8ERR_FSTREAM_SEEK_FAILURE;
		goto end;
	}
	
	buffer = malloc((size_t) file_size);
	
	if (buffer == NULL) {
		err = M3U8ERR_MEMORY_ALLOCATE_FAILURE;
		goto end;
	}
	
	rsize = fstream_read(stream, buffer, (size_t) file_size);
	
	if (rsize == -1) {
		err = M3U8ERR_FSTREAM_READ_FAILURE;
		goto end;
	}
	
	cacerts->data = buffer;
	cacerts->len = (size_t) rsize;
	
	end:;
	
	fstream_close(stream);
	
	return err;
	
}

int sslcerts_load_certificates(CURL* const curl) {
	
	int err = M3U8ERR_SUCCESS;
	CURLcode code = CURLE_OK;
	
	size_t index = 0;
	
	char* app_filename = NULL;
	char* name = NULL;
	
	code = curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
	
	if (code != CURLE_OK) {
		err = M3U8ERR_CURL_SETOPT_FAILURE;
		goto end;
	}
	
	if (cacerts != NULL) {
		code = curl_easy_setopt(curl, CURLOPT_CAINFO_BLOB, cacerts);
		
		if (code != CURLE_OK) {
			err = M3U8ERR_CURL_SETOPT_FAILURE;
		}
		
		goto end;
	}
	
	cacerts = malloc(sizeof(*cacerts));
	
	if (cacerts == NULL) {
		err = M3U8ERR_MEMORY_ALLOCATE_FAILURE;
		goto end;
	}
	
	for (index = 0; index < sizeof(SSL_CERTIFICATE_LOCATIONS) / sizeof(*SSL_CERTIFICATE_LOCATIONS); index++) {
		const char* const location = SSL_CERTIFICATE_LOCATIONS[index];
		
		err = sslcerts_load_file(location);
		
		if (err != M3U8ERR_SUCCESS) {
			continue;
		}
		
		code = curl_easy_setopt(curl, CURLOPT_CAINFO_BLOB, cacerts);
		
		if (code != CURLE_OK) {
			err = M3U8ERR_CURL_SETOPT_FAILURE;
		}
		
		goto end;
	}
	
	app_filename = get_app_filename();
	
	if (app_filename == NULL) {
		err = M3U8ERR_GET_APP_FILENAME_FAILURE;
		goto end;
	}
	
	name = malloc(strlen(app_filename) + strlen(BUILTIN_CA_CERT_LOCATION) + 1);
	
	if (name == NULL) {
		err = M3U8ERR_MEMORY_ALLOCATE_FAILURE;
		goto end;
	}
	
	get_parent_directory(app_filename, name, 2);
	
	strcat(name, BUILTIN_CA_CERT_LOCATION);
	
	err = sslcerts_load_file(name);
	
	if (err != M3U8ERR_SUCCESS) {
		goto end;
	}
	
	end:;
	
	free(app_filename);
	free(name);
	
	return err;
	
}

void sslcerts_unload_certificates(void) {
	
	if (cacerts == NULL) {
		return;
	}
	
	free(cacerts->data);
	cacerts->len = 0;
	
	free(cacerts);
	cacerts = NULL;
	
}
