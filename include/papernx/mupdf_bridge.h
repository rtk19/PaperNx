#pragma once
#ifdef __cplusplus
extern "C" {
#endif
typedef struct PxDocument PxDocument;
typedef struct PxImage { int width,height,stride;unsigned char *rgb; } PxImage;
PxDocument *px_document_open(const char *path,int width,int height,float em,const char *css,char *error,int capacity);
int px_document_pages(PxDocument *doc);
int px_document_is_epub(PxDocument *doc);
int px_document_relayout(PxDocument *doc,int width,int height,float em,const char *css,char *error,int capacity);
int px_document_metadata(PxDocument *doc,const char *key,char *out,int capacity);
int px_document_render(PxDocument *doc,int page,int max_width,int max_height,PxImage *image,char *error,int capacity);
void px_image_free(PxImage *image);
void px_document_close(PxDocument *doc);
#ifdef __cplusplus
}
#endif
