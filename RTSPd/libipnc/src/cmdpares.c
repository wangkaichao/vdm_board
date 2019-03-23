/*
*2012.7.5  by Aaron.qiu
*/

#include <stdio.h>
#include <string.h>

#include "cmdpares.h"

#define _PROC_DEBUG 	1

#if _PROC_DEBUG
#define _Dbg(fmt, args...) 	printf("%s:%d "fmt,__FILE__,__LINE__,args)
#else
#define _Dbg(fmt, args...) 
#endif

/*
 * É¨Ãè×Ö·û´®£¬Óöµ½'+' = ' '£¬'%N1N2' = 1byte,  stop until '=','&',0,'#'
 * PostIn£º´ý½âÎöµÄ×Ö·û´®
 * pParseIndex£º´ý½âÎö×Ö·û´®µÄÏÂ±ê
*/
char *ParseVars(char PostIn[], int *pParseIndex)
{
  int  out;
  int  in;
  char hexch;
  char hexval[3];
  int start;
  char ch;

  in = *pParseIndex;

  hexval[2] = '\0';
  if((in == 0) && (PostIn[0]=='$'))
  	out = in =  1;
  else if(in == 0)
  	out = 0;
  else if(in == -1)
  	return NULL;
  else
  	out = ++in;

  start = in;
  while (((ch = PostIn[in]) != '=') && (ch != '&') && (ch != '\0') && (ch != '#'))
  {
    if (ch == '+')
      PostIn[out++] = ' ';
    else if (ch == '%')
    {
      hexval[0] = PostIn[++in];
      hexval[1] = PostIn[++in];
      hexch = (char) strtol(hexval, NULL, 16);
      PostIn[out++] = hexch;
     }
     else
      PostIn[out++] = ch;
	 
    in++;
  }

  if((ch == '\0')||(ch == '#'))
      in = -1;

  PostIn[out++] = '\0';

  *pParseIndex = in;

  return (&PostIn[start]);
}
























