#include "papernx/mupdf_bridge.h"
#include <mupdf/fitz.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
struct PxDocument {fz_context *ctx;fz_document *doc;int pages,epub;};
static void report(char *dst,int cap,const char *msg){if(dst&&cap>0)snprintf(dst,(size_t)cap,"%s",msg?msg:"Document error");}
void px_document_close(PxDocument *d){
 if(d){if(d->ctx){fz_drop_document(d->ctx,d->doc);fz_drop_context(d->ctx);}free(d);}
}
PxDocument *px_document_open(const char *path,int w,int h,float em,const char *css,char *err,int cap){
 PxDocument *d=(PxDocument*)calloc(1,sizeof(*d));
 if(!d){report(err,cap,"Out of memory");return NULL;}
 d->ctx=fz_new_context(NULL,NULL,128<<20);
 if(!d->ctx){report(err,cap,"Cannot create MuPDF context");free(d);return NULL;}
 fz_try(d->ctx){
  fz_register_document_handlers(d->ctx);
  d->doc=fz_open_document(d->ctx,path);
  if(fz_needs_password(d->ctx,d->doc))fz_throw(d->ctx,FZ_ERROR_GENERIC,"Password-protected document is unsupported");
  d->epub=fz_is_document_reflowable(d->ctx,d->doc);
  if(d->epub){fz_style_document(d->ctx,d->doc,1,css?css:"");fz_layout_document(d->ctx,d->doc,(float)w,(float)h,em);}
  d->pages=fz_count_pages(d->ctx,d->doc);
 }
 fz_catch(d->ctx){report(err,cap,fz_caught_message(d->ctx));px_document_close(d);return NULL;}
 return d;
}
int px_document_pages(PxDocument*d){return d?d->pages:0;}
int px_document_is_epub(PxDocument*d){return d?d->epub:0;}
int px_document_relayout(PxDocument*d,int w,int h,float em,const char *css,char*err,int cap){
 if(!d||!d->epub)return 0;
 int ok=1;
 fz_try(d->ctx){fz_style_document(d->ctx,d->doc,1,css?css:"");fz_layout_document(d->ctx,d->doc,(float)w,(float)h,em);d->pages=fz_count_pages(d->ctx,d->doc);}
 fz_catch(d->ctx){report(err,cap,fz_caught_message(d->ctx));ok=0;}
 return ok;
}
int px_document_metadata(PxDocument*d,const char*key,char*out,int cap){
 if(!d||!out||cap<=0)return 0;int result=0;out[0]=0;
 fz_try(d->ctx){result=fz_lookup_metadata(d->ctx,d->doc,key,out,cap);}
 fz_catch(d->ctx){out[0]=0;result=0;}
 return result;
}
int px_document_render(PxDocument*d,int index,int max_w,int max_h,PxImage*out,char*err,int cap){
 if(!d||!out||index<0||index>=d->pages||max_w<=0||max_h<=0)return 0;
 memset(out,0,sizeof(*out));fz_page *page=NULL;fz_pixmap *pix=NULL;int ok=1;
 fz_var(page);fz_var(pix);
 fz_try(d->ctx){
  page=fz_load_page(d->ctx,d->doc,index);
  fz_rect bounds=fz_bound_page(d->ctx,page);
  const float w=bounds.x1-bounds.x0,h=bounds.y1-bounds.y0;
  if(!(w>0&&h>0&&isfinite(w)&&isfinite(h)))fz_throw(d->ctx,FZ_ERROR_GENERIC,"Invalid page dimensions");
  const float scale=fminf((float)max_w/w,(float)max_h/h);
  if(!(scale>0&&isfinite(scale)))fz_throw(d->ctx,FZ_ERROR_GENERIC,"Invalid page scale");
  pix=fz_new_pixmap_from_page(d->ctx,page,fz_scale(scale,scale),fz_device_rgb(d->ctx),0);
  int pw=fz_pixmap_width(d->ctx,pix),ph=fz_pixmap_height(d->ctx,pix),n=fz_pixmap_components(d->ctx,pix),stride=fz_pixmap_stride(d->ctx,pix);
  if(pw<1||ph<1||pw>4096||ph>4096||n!=3)fz_throw(d->ctx,FZ_ERROR_GENERIC,"Render budget exceeded");
  out->rgb=(unsigned char*)malloc((size_t)pw*(size_t)ph*3);
  if(!out->rgb)fz_throw(d->ctx,FZ_ERROR_GENERIC,"Allocation failed");
  const unsigned char *samples=fz_pixmap_samples(d->ctx,pix);
  for(int y=0;y<ph;++y)memcpy(out->rgb+(size_t)y*pw*3,samples+(size_t)y*stride,(size_t)pw*3);
  out->width=pw;out->height=ph;out->stride=pw*3;
 }
 fz_catch(d->ctx){report(err,cap,fz_caught_message(d->ctx));px_image_free(out);ok=0;}
 fz_drop_pixmap(d->ctx,pix);fz_drop_page(d->ctx,page);
 return ok;
}
void px_image_free(PxImage*i){if(i){free(i->rgb);memset(i,0,sizeof(*i));}}
