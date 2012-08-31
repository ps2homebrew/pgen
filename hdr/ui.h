#ifndef _UI_H
#define _UI_H

int ui_init();
int ui_loop(void);
void ui_line(int line);
void ui_endfield(void);
void ui_final(void);
void ui_log_debug3(const char *text, ...);
void ui_log_debug2(const char *text, ...);
void ui_log_debug1(const char *text, ...);
void ui_log_user(const char *text, ...);
void ui_log_verbose(const char *text, ...);
void ui_log_normal(const char *text, ...);
void ui_log_critical(const char *text, ...);
void ui_log_request(const char *text, ...);
void ui_err(const char *text, ...);
void ui_musiclog(uint8 *data, unsigned int length);

#endif
