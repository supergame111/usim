#ifndef USIM_DECODE_H
#define USIM_DECODE_H

extern int read_prom_files(void);
extern int show_prom(void);

extern void disassemble_m_src(ucw_t u, int m_src);
extern void disassemble_dest(int dest);
extern void disassemble_ucode_loc(ucw_t u);
extern void disassemble_prom(void);

extern char *find_function_name(unsigned int the_lc);

extern void show_list(void);

#endif
