#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>

#include "fs/fstream.h"
#include "fs/getexec.h"
#include "sslcerts.h"
#include "fs/sep.h"
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
	"/etc/openssl/certs/ca-certificates.crt",
	"/usr/local/share/certs/ca-root-nss.crt",
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
	PATHSEP_M
	"etc"
	PATHSEP_M
	"tls"
	PATHSEP_M
	"cert.pem";

struct curl_blob* cacerts = NULL;

static int sslcerts_load_file(const char* const name) {
	
	fstream_t* stream = NULL;
	char* buffer = NULL;
	
	long int file_size = 0;
	ssize_t rsize = 0;
	
	int err = SSLCERTS_SUCCESS;
	
	stream = fstream_open(name, FSTREAM_READ);
	
	if (stream == NULL) {
		err = SSLCERTS_ERROR;
		goto end;
	}
	
	file_size = fsream_size(stream);
	
	if (file_size == FSTREAM_ERROR) {
		err = SSLCERTS_ERROR;
		goto end;
	}
	
	buffer = malloc((size_t) file_size);
	
	if (buffer == NULL) {
		err = SSLCERTS_ERROR;
		goto end;
	}
	
	rsize = fstream_read(stream, buffer, (size_t) file_size);
	
	if (rsize == FSTREAM_ERROR) {
		err = SSLCERTS_ERROR;
		goto end;
	}
	
	cacerts->data = buffer;
	cacerts->len = (size_t) rsize;
	
	end:;
	
	fstream_close(stream);
	
	return err;
	
}

int sslcerts_load_certificates(CURL* const curl) {
	
	int err = SSLCERTS_SUCCESS;
	
	CURLcode code = CURLE_OK;
	
	size_t index = 0;
	
	const char* location = NULL;
	
	char* app_directory = NULL;
	char* name = NULL;
	
	code = curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
	
	if (code != CURLE_OK) {
		err = SSLCERTS_ERROR;
		goto end;
	}
	
	if (cacerts != NULL) {
		code = curl_easy_setopt(curl, CURLOPT_CAINFO_BLOB, cacerts);
		
		if (code != CURLE_OK) {
			err = SSLCERTS_ERROR;
		}
		
		goto end;
	}
	
	cacerts = malloc(sizeof(*cacerts));
	
	if (cacerts == NULL) {
		err = SSLCERTS_ERROR;
		goto end;
	}
	
	for (index = 0; index < sizeof(SSL_CERTIFICATE_LOCATIONS) / sizeof(*SSL_CERTIFICATE_LOCATIONS); index++) {
		location = SSL_CERTIFICATE_LOCATIONS[index];
		
		err = sslcerts_load_file(location);
		
		if (err != SSLCERTS_SUCCESS) {
			continue;
		}
		
		code = curl_easy_setopt(curl, CURLOPT_CAINFO_BLOB, cacerts);
		
		if (code != CURLE_OK) {
			err = SSLCERTS_ERROR;
		}
		
		goto end;
	}
	
	app_directory = get_app_directory();
	
	if (app_directory == NULL) {
		err = SSLCERTS_ERROR;
		goto end;
	}
	
	name = malloc(strlen(app_directory) + strlen(BUILTIN_CA_CERT_LOCATION) + 1);
	
	if (name == NULL) {
		err = SSLCERTS_ERROR;
		goto end;
	}
	
	strcpy(name, app_directory);
	strcat(name, BUILTIN_CA_CERT_LOCATION);
	
	err = sslcerts_load_file(name);
	
	if (err != SSLCERTS_SUCCESS) {
		goto end;
	}
	
	code = curl_easy_setopt(curl, CURLOPT_CAINFO_BLOB, cacerts);
	
	if (code != CURLE_OK) {
		err = SSLCERTS_ERROR;
	}
	
	end:;
	
	free(app_directory);
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
