
struct buffer {
	size_t size;
	size_t offset;
	char* data;
};

typedef struct buffer buffer_t;

int buffer_init(buffer_t* const buffer, const size_t size);
int buffer_append(buffer_t* const buffer, const char* const data, const size_t size);
void buffer_free(buffer_t* const buffer);
