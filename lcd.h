
/* To control LCD By http://www.nicovideo.jp/watch/sm15586503 */

void lcd_init (void);
void lcd_out (char, char);
void lcd_cmd (char);
void lcd_data (char);
void lcd_str (char*);
void lcd_pos (char,char);
void lcd_clear (void);
