#include "klib.h"
#include <stdarg.h>

//#ifndef __ISA_NATIVE__
#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
  //char* test = "T";
	//_putc(*test);
	//_putc('\n');
	char pbuf[1024];
  va_list ap;
  int i;
  va_start(ap,fmt);
  i=vsprintf(pbuf,fmt,ap);
  va_end(ap);
  int j=0;
  while(pbuf[j]!='\0'){
     _putc(pbuf[j]);
	 j++;
  }
  _putc('\0');
  return i;
}

static char numtrans[16]="0123456789ABCDEF";

int vsprintf(char *out, const char *fmt, va_list ap) {
  char * str;
  const char* s;
  int num;
	unsigned int num32;
	unsigned long int num64;
  for(str=out;*fmt;fmt++){
	  if(*fmt!='%'){
		  *str++=*fmt;
		  continue;
	  }
	  ++fmt;
	  switch(*fmt){
		  case 's':
			  s=va_arg(ap,char *);
			  while(*s)
				  *str++=*s++;
			  break;
		 case 'd':
			  num=va_arg(ap,long);
			  int i=0;char tmp[32];
			  if(num==0){
				  *str++='0';
				  break;
			  }
			//	if(num < 0)
			//		num = -num;
			  while(num){
				  tmp[i]=numtrans[((unsigned long)num)%10];
				  num/=10;
				  i++;
			  }
			  while(i){
				  i--;
				  *str++=tmp[i];
			  }
			  break;
		  case 'I':
				++fmt;
				if(*fmt == '6'){
					fmt++;++fmt;
					num64=va_arg(ap,unsigned long int);
			  	int i64=0;char tmp64[20];
			  		if(num64==0){
				  	*str++='0';
				  	break;
			  	}
			  	while(num64){
				  	tmp64[i64]=numtrans[num64%10];
				  	num64/=10;
				  	i64++;
			  	}
			  	while(i64){
				  	i64--;
				  	*str++=tmp64[i64];
			  	}
				}
				else if(*fmt == '3'){
					++fmt;++fmt;
					num32=va_arg(ap,unsigned int);
			  	int i32=0;char tmp32[20];
			  	if(num32==0){
				  	*str++='0';
				  	break;
			  	}
			  	while(num32){
				  	tmp32[i32]=numtrans[num32%10];
				  	num32/=10;
				  	i32++;
			  	}
			  	while(i32){
				  	i32--;
				  	*str++=tmp32[i32];
			  	}
				}
				break;
			case 'x':
				num=va_arg(ap,unsigned int);
			  int i2=0;char tmp2[32];
			  if(num==0){
				  *str++='0';
				  break;
			  }
			  while(num){
				  tmp2[i2]=numtrans[num%16];
				  num/=16;
				  i2++;
			  }
			  while(i2){
				  i2--;
				  *str++=tmp2[i2];
			  }
			  break;
		default:;
	  }
  }
  *str='\0';
  return str-out;
}

int sprintf(char *out, const char *fmt, ...) {
    va_list ap;
	int i;
	va_start(ap,fmt);
    i=vsprintf(out,fmt,ap);
	va_end(ap);
  	return i;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  return 0;
}

#endif
