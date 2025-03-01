#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

#define SSD1306_WIDTH 128
#define SSD1306_HEIGHT 64

#define SPI_DC_PIN_NUM  (352+21)    // SD1_DO SPI2_SDI  0X03001098 pwr_gpio21
#define SPI_RST_PIN_NUM (480+29)    // JTAG_TDO 0X03001050 XGPIOA29

unsigned char buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];

static void init_gpio(int num, int direction, int value)
{
    char cmd[64] = {0};
    sprintf(cmd, "echo %d > /sys/class/gpio/export", num);
    printf("%s\n", cmd);
    system(cmd);

    if(direction == 0) // input
    {
        memset(cmd, 0, 64);
        sprintf(cmd, "echo in > /sys/class/gpio/gpio%d/direction", num);
        printf("%s\n", cmd);
        system(cmd);
        return;
    } else {    // output
        memset(cmd, 0, 64);
        sprintf(cmd, "echo out > /sys/class/gpio/gpio%d/direction", num);
        printf("%s\n", cmd);
        system(cmd);

        memset(cmd, 0, 64);
        sprintf(cmd, "echo %d > /sys/class/gpio/gpio%d/value", value, num);
        printf("%s\n", cmd);
        system(cmd);
        return;
    }
}

static int gpio_output(int num, int value)
{
    int fd = -1;
    int ret = -1;
    char filename[64] = {0};
    char str_value[4] = {0};

    // open file
    sprintf(filename, "/sys/class/gpio/gpio%d/value", num);
    fd = open(filename, O_WRONLY);
    if (fd < 0) {
        perror("open gpio error: \n");
    }

    // write value
    sprintf(str_value, "%d", value);
    ret = write(fd, str_value, 1);
    close(fd);

    return fd;
}

void ssd1306_init(int fd)
{
    // 初始化SSD1306屏幕
    unsigned char init_seq[] = {
        0xAE, // 关闭显示
        0xD5, 0x80, // 设置时钟分频因子
        0xA8, 0x3F, // 设置多路复用率
        0xD3, 0x00, // 设置显示偏移
        0x40, // 设置起始行
        0x8D, 0x14, // 设置电荷泵
        0x20, 0x00, // 设置内存地址模式
        0xA1, // 设置段重定向
        0xC8, // 设置COM扫描方向
        0xDA, 0x12, // 设置COM硬件引脚配置
        0x81, 0xCF, // 设置对比度
        0xD9, 0xF1, // 设置预充电周期
        0xDB, 0x40, // 设置VCOMH
        0xA4, // 关闭全局显示
        0xA6, // 设置正常/反相显示
        0xAF, // 打开显示
    };

    struct spi_ioc_transfer tr;
    tr.tx_buf = (unsigned long)init_seq;
    tr.rx_buf = 0;
    tr.len = sizeof(init_seq);
    tr.delay_usecs = 0;
    tr.speed_hz = 1000000;
    tr.bits_per_word = 8;

    if (ioctl(fd, SPI_IOC_MESSAGE(1), &tr) < 0) {
        perror("ioctl");
        exit(1);
    }
}

void ssd1306_display(int fd)
{
    // 显示缓冲区中的内容
    unsigned char display_seq[] = {
        0x21, 0x00, 0x7F, // 设置列地址范围
        0x22, 0x00, 0x07, // 设置页地址范围
    };

    struct spi_ioc_transfer tr[2];
    tr[0].tx_buf = (unsigned long)display_seq;
    tr[0].rx_buf = 0;
    tr[0].len = sizeof(display_seq);
    tr[0].delay_usecs = 0;
    tr[0].speed_hz = 1000000;
    tr[0].bits_per_word = 8;

    tr[1].tx_buf = (unsigned long)buffer;
    tr[1].rx_buf = 0;
    tr[1].len = sizeof(buffer);
    tr[1].delay_usecs = 0;
    tr[1].speed_hz = 1000000;
    tr[1].bits_per_word = 8;

    if (ioctl(fd, SPI_IOC_MESSAGE(2), &tr) < 0) {
        perror("ioctl");
        exit(1);
    }
}

int main(int argc, char *argv[])
{
    int fd;
    struct spi_ioc_transfer tr;

    // init spio
    init_gpio(SPI_DC_PIN_NUM, 1, 1);
    init_gpio(SPI_RST_PIN_NUM, 1, 1);

    // 打开SPI设备节点
    fd = open("/dev/spidev0.0", O_RDWR);
    if (fd < 0) {
        perror("open");
        exit(1);
    }

    // 配置SPI传输参数
    tr.tx_buf = 0;
    tr.rx_buf = 0;
    tr.len = 0;
    tr.delay_usecs = 0;
    tr.speed_hz = 1000000;
    tr.bits_per_word = 8;

    if (ioctl(fd, SPI_IOC_MESSAGE(1), &tr) < 0) {
        perror("ioctl");
        exit(1);
    }

    // 初始化SSD1306屏幕
    ssd1306_init(fd);

    // 在缓冲区中绘制图形
    // ...

    // 显示缓冲区中的内容
    ssd1306_display(fd);

    // 关闭SPI设备节点
    close(fd);

    return 0;
}