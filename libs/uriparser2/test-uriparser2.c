#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#undef NDEBUG
#include <assert.h>

#include "uriparser2.h"

#if 0
static void print(URI *a) {
	if (a) {
		printf(
			"scheme  =%s\n"
			"user    =%s\n"
			"pass    =%s\n"
			"host    =%s\n"
			"port    =%d\n"
			"path    =%s\n"
			"query   =%s\n"
			"fragment=%s\n",
			a->scheme_, a->user_, a->pass_, a->host_,
			a->port_, a->path_, a->query_, a->fragment_);
	} else {
		printf("null");
	}
}

static URI *uri_parse2(const char *uri) {
	URI *a = uri_parse(uri);
	print(a);
	return a;
}
#define uri_parse(s)	uri_parse2(s)
#endif

static void simple_test(void) {
	URI *a = uri_parse("http://www.google.com/search?q=uriparser#top");
	assert(a);
	assert(a->port_ == 0);
	assert(!strcmp(a->scheme_,   "http"));
	assert(!strcmp(a->host_,     "www.google.com"));
	assert(!strcmp(a->path_,     "/search"));
	assert(!strcmp(a->query_,    "q=uriparser"));
	assert(!strcmp(a->fragment_, "top"));
}

static void multi_segment_path(void) {
	URI *a = uri_parse("http://www.example.com/foo/bar/baz");
	assert(!strcmp(a->path_, "/foo/bar/baz"));
}

static void file_path(void) {
	URI *a = uri_parse("file:///foo/bar/baz");
	assert(a->host_ == 0);
	assert(!strcmp(a->path_, "/foo/bar/baz"));
}

static void port_number(void) {
	URI *a = uri_parse("http://localhost:8080/");
	assert(a->port_ == 8080);
	assert(!strcmp(a->host_, "localhost"));
	assert(!strcmp(a->path_, "/"));
}

static void user_info(void) {
	URI *a = uri_parse("http://foo:bar@localhost:8080/");
	assert(!strcmp(a->user_, "foo"));
	assert(!strcmp(a->pass_, "bar"));
}

static void user_info_only_user(void) {
	URI *a = uri_parse("http://foo@localhost:8080/");
	assert(!strcmp(a->user_, "foo"));
	assert(a->pass_ == 0);
}

static void user_info_only_pass(void) {
	URI *a = uri_parse("http://:bar@localhost:8080/");
	assert(a->user_ == 0);
	assert(!strcmp(a->pass_, "bar"));
}

static void recomposed_equals_original_url(void) {
	const char *uri = "http://foo:bar@example.com:8080/path/to/resource?q=hello+world&ln=en#top";
	URI *a = uri_parse(uri);
	char *uri2 = uri_build(a);
	assert(!strcmp(uri, uri2));
}

static void equal(void) {
	const char *uri = "http://www.google.com/search?q=uriparser2&ln=en#top";
	URI *a = uri_parse(uri);
	URI *b = uri_parse(uri);
	assert(0 == uri_compare(a, b));
}

int main(void) {
	simple_test();
	multi_segment_path();
	file_path();
	port_number();
	user_info();
	user_info_only_user();
	user_info_only_pass();
	recomposed_equals_original_url();
	equal();
	puts("All tests OK.");
	return 0;
}
