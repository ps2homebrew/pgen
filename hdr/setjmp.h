#ifndef _SETJMP_H
#define _SETJMP_H

#define _JBLEN 23
#define _JBTYPE long long

typedef	_JBTYPE jmp_buf[_JBLEN];

extern int setjmp (jmp_buf __env);
extern void longjmp (jmp_buf __env, int __val);

#endif
