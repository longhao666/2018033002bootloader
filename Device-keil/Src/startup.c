// 没有固件的时候，直接跳转到booLoader
extern int bootLoader(void);

int main(void)
{
  bootLoader();
}
