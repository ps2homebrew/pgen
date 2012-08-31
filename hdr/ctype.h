#ifndef _CTYPE_H_
#ifdef __cplusplus
extern "C" {
#endif
#define _CTYPE_H_

#define   isalnum(c)  (isalpha(c) || isdigit(c))
#define   isalpha(c)  (islower(c) || isupper(c))
#define   isascii(c)  (((c) >= 0) && ((c) <= 0x7f))
#define   iscntrl(c)  ((((c) >= 0) && ((c) <= 0x1f)) || ((c) == 0x7f))
#define   isdigit(c)  (((c) >= '0') && ((c) <= '9'))
#define   islower(c)  (((c) >= 'a') && ((c) <= 'z'))
#define   isprint(c)  (((c) >= 0x20) && ((c) <= 0x7e))
#define   ispunct(c)  ((((c) >= 0x20) && ((c) <= 0x2f)) || \
                       (((c) >= 0x3a) && ((c) <= 0x40)) || \
                       (((c) >= 0x5b) && ((c) <= 0x60)) || \
                       (((c) >= 0x7b) && ((c) <= 0x7e)))
#define   isspace(c)  (((c) == 0x09) || ((c) == 0x0a) || \
                       ((c) == 0x0b) || ((c) == 0x0c) || \
                       ((c) == 0x0d) || ((c) == 0x20))
#define   isupper(c)  (((c) >= 'A') && ((c) <= 'Z'))
#define   isxdigit(c) (isdigit(c) || (((c) >= 'a') && ((c) <= 'f')) || \
                       (((c) >= 'A') && ((c) <= 'F')))
#define   tolower(c)  (isupper(c) ? ((c) + 'a' - 'A'): (c))
#define   toupper(c)  (islower(c) ? ((c) - 'a' + 'A'): (c))

#ifdef __cplusplus
}
#endif
#endif /* _CTYPE_H_ */
