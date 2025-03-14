#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <istream>
#include <unistd.h>
#include "CommApi.h"
#include "proxy/ProxyMotion.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h> 
#include <sys/stat.h>
#include <signal.h>
#include <getopt.h>
#define termios asmtermios
#include <asm/termios.h>
#undef  termios
#include <linux/serial.h>
#include <termios.h>
extern "C" {
    extern int ioctl(int d, int request, ...);
}

bool connectIPC(Hsc3::Comm::CommApi &cmApi, std::string strIP, uint16_t uPort);
bool disconnectIPC(Hsc3::Comm::CommApi &cmApi);
void waitDone(Hsc3::Proxy::ProxyMotion &pMot);
void loadPosData(GeneralPos &generalPos, double axis0, double axis1, double axis2, double axis3, bool isJoint, int8_t ufNum);
int libtty_setcustombaudrate(int fd, int baudrate);
int libtty_open(const char *devname);
int libtty_setopt(int fd, int speed, int databits, int stopbits, char parity, char hardflow);
void printvec(vector<uint8_t> v);

// std::vector<double> &get_coordinate(std::vector<double> &pre_coordinate, std::fstream &file);
// std::vector<double>& transform(std::vector<double> before, std::vector<double>& after);

// std::vector<double> pre_coordinate, coordinate;
// std::fstream fs;

// 用来统计工作的次数，到达设定的次数后停止
static int count = 0;

int main()
{
    Hsc3::Comm::CommApi cmApi("");
    Hsc3::Proxy::ProxyMotion pMot(&cmApi);

    GeneralPos base_point, target_point;
    loadPosData(base_point, 0, 0, 0, 0, false, 0);       // 工作原点
    loadPosData(target_point, 50, 50, 50, 50, false, 0); // 药浴和乳刷点

    if (connectIPC(cmApi, "10.10.56.214", 23234))
    {
        // 设置手动模式
        pMot.setOpMode(OP_T1);
        // 设置手动运行倍率
        pMot.setJogVord(40);
        // 设置坐标系  FRAME_BASE: 用户坐标系(工件坐标系)  FRAME_JOINT: 关节坐标系
        pMot.setWorkFrame(0, FRAME_BASE);
        // 选择用户坐标系(工件坐标系)  setWorkpieceNum(int8_t gpId, int8_t num): gpId 组号，num 工件坐标索引(0..15)
        pMot.setWorkpieceNum(0, 0);
        // 机器人上使能
        pMot.setGpEn(0, true);
        sleep(2);

        // 首先运动到原点，属于准备工作，防止机械臂刚上来位置不正确，发生事故
        pMot.moveTo(0, base_point, true); // true: 直线运动, false: 关节运动
        waitDone(pMot);
        std::cout << "已运动到原点" << std::endl;
        
        // 开始工作前，需要检测一次奶牛腿部的位置，防止碰撞；之后的每次工作就不需要检测了，因为转盘转动的速度是恒定的

        // 需要另开一个线程，进行实时检测，防止碰撞

        // 正式工作流程
        while (count < 5)
        {
            pMot.moveTo(0, target_point, true);
            waitDone(pMot);
            std::cout << "已运动到药浴点" << std::endl;

            pMot.moveTo(0, base_point, true);
            waitDone(pMot);
            std::cout << "已返回到工作原点" << std::endl;
            ++count;
        }
    }
    else
    {
        std::cout << "连接失败" << std::endl;
    }

    // 机器人下使能
    pMot.setGpEn(0, false);
    // 断连
    disconnectIPC(cmApi);

    system("pause");
    return 0;
}

/************************************************************************/
/*
 * @brief 连接IPC
 * @param cmApi:通信客户端对象，建议在自定义的函数中如果需要传递客户端对象，都使用引用传递。
 * @param strIP:IP，控制器默认IP是"10.10.56.214"
 * @param uPort:端口号,固定端口号:23234
 */
/************************************************************************/
bool connectIPC(Hsc3::Comm::CommApi &cmApi, std::string strIP, uint16_t uPort)
{
    // 1.设置非自动重连模式,连接前调用。
    cmApi.setAutoConn(false);
    // 2.连接
    Hsc3::Comm::HMCErrCode ret = cmApi.connect(strIP, uPort);
    if (ret != 0)
    {
        std::cout << "CommApi::connect : ret = " << ret << std::endl;
        // printf("CommApi::connect :ret=%lld\n",ret);
    }
    // 3.查询是否连接
    if (cmApi.isConnected())
    {
        std::cout << "连接成功" << std::endl;
        return true;
    }
    else
    {
        std::cout << "连接失败" << std::endl;
        return false;
    }
}

/************************************************************************/
/*
 * @brief 断开与IPC的连接
 * @param cmApi:通信客户端对象
 */
/************************************************************************/
bool disconnectIPC(Hsc3::Comm::CommApi &cmApi)
{
    Hsc3::Comm::HMCErrCode ret = cmApi.disconnect();
    sleep(1);
    if (cmApi.isConnected())
    {
        return false;
    }
    else
    {
        return true;
    }
}

/************************************************************************/
/*
 * @brief 等待运动停止，只能用于手动模式下用于检测是否处于静止状态或错误状态。
 * @param pMot:运动功能代理
 */
/************************************************************************/
void waitDone(Hsc3::Proxy::ProxyMotion &pMot)
{
    ManState manualState = MAN_STATE_MAX;
    //此延时是为了防止机器人还未进入运动状态
    sleep(2);
    while (1)
    {
        pMot.getManualStat(manualState);
        if (manualState == MAN_STATE_WAIT || manualState == MAN_STATE_ERROR)
        {
            if (manualState == MAN_STATE_ERROR)
            {
                cout << "移动错误" << endl;
            }
            if (manualState == MAN_STATE_WAIT)
            {
                cout << "完成移动" << endl;
            }
            break;
        }
    }
}

// isJoint: 是否关节点，和moveTo()函数的第3个参数(isLinear: 是否直线运动、关节运动)有关
// isJoint是true，则isLinear为false(是关节点，则关节运动)；isJoint是false，则isJoint为true(不是关节点，则直线运动)
// ufNum: 工件坐标系(用户坐标系)
void loadPosData(GeneralPos &generalPos, double axis0, double axis1, double axis2, double axis3, bool isJoint, int8_t ufNum)
{
    generalPos.config = 1048576;
    generalPos.isJoint = isJoint; // 是否关节点
    generalPos.ufNum = ufNum;
    generalPos.utNum = -1; // 工具号
    generalPos.vecPos.push_back(axis0);
    generalPos.vecPos.push_back(axis1);
    generalPos.vecPos.push_back(axis2);
    generalPos.vecPos.push_back(axis3);
    generalPos.vecPos.push_back(0);
    generalPos.vecPos.push_back(0);
    generalPos.vecPos.push_back(0);
    generalPos.vecPos.push_back(0);
    generalPos.vecPos.push_back(0);
}

/*
std::vector<double> &get_coordinate(std::vector<double> &pre_coordinate, std::fstream &file)
{
    coordinate.clear();
    file.open("/home/nvidia/Desktop/yolov5_d435i_detection-main/Binocular_camera.txt", ios::in);
    std::string str;
    getline(file, str);
    for (int i = 0; i < str.size(); i++)
    {
        if (str[i] == ' ')
        {
            pre_coordinate.push_back(stod(str.substr(j, i - j)));
            j = i + 1;
        }
    }
    file.close();
    return pre_coordinate;
}

std::vector<double>& transform(std::vector<double> before, std::vector<double>& after)
{
}
*/

int libtty_setcustombaudrate(int fd, int baudrate)
{
    struct termios2 tio;

    if (ioctl(fd, TCGETS2, &tio))
    {
        perror("TCGETS2");
        return -1;
    }

    tio.c_cflag &= ~CBAUD;
    tio.c_cflag |= BOTHER;
    tio.c_ispeed = baudrate;
    tio.c_ospeed = baudrate;

    if (ioctl(fd, TCSETS2, &tio))
    {
        perror("TCSETS2");
        return -1;
    }

    if (ioctl(fd, TCGETS2, &tio))
    {
        perror("TCGETS2");
        return -1;
    }

    return 0;
}

int libtty_open(const char *devname)
{
    int fd = open(devname, O_RDWR | O_NOCTTY | O_NDELAY | O_TRUNC);
    int flags = 0;

    if (fd < 0)
    {
        perror("open device failed");
        return -1;
    }

    flags = fcntl(fd, F_GETFL, 0);
    flags &= ~O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flags) < 0)
    {
        printf("fcntl failed.\n");
        return -1;
    }

    if (isatty(fd) == 0)
    {
        printf("not tty device.\n");
        return -1;
    }
    else
        printf("tty device test ok.\n");

    return fd;
}

int libtty_setopt(int fd, int speed, int databits, int stopbits, char parity, char hardflow)
{
    struct termios newtio;
    struct termios oldtio;
    int i;

    bzero(&newtio, sizeof(newtio));
    bzero(&oldtio, sizeof(oldtio));

    if (tcgetattr(fd, &oldtio) != 0)
    {
        perror("tcgetattr");
        return -1;
    }
    newtio.c_cflag |= CLOCAL | CREAD;
    newtio.c_cflag &= ~CSIZE;

    /* set data bits */
    switch (databits)
    {
    case 5:
        newtio.c_cflag |= CS5;
        break;
    case 6:
        newtio.c_cflag |= CS6;
        break;
    case 7:
        newtio.c_cflag |= CS7;
        break;
    case 8:
        newtio.c_cflag |= CS8;
        break;
    default:
        fprintf(stderr, "unsupported data size\n");
        return -1;
    }

    /* set parity */
    switch (parity)
    {
    case 'n':
    case 'N':
        newtio.c_cflag &= ~PARENB; /* Clear parity enable */
        newtio.c_iflag &= ~INPCK;  /* Disable input parity check */
        break;
    case 'o':
    case 'O':
        newtio.c_cflag |= (PARODD | PARENB); /* Odd parity instead of even */
        newtio.c_iflag |= INPCK;             /* Enable input parity check */
        break;
    case 'e':
    case 'E':
        newtio.c_cflag |= PARENB;  /* Enable parity */
        newtio.c_cflag &= ~PARODD; /* Even parity instead of odd */
        newtio.c_iflag |= INPCK;   /* Enable input parity check */
        break;
    default:
        fprintf(stderr, "unsupported parity\n");
        return -1;
    }

    /* set stop bits */
    switch (stopbits)
    {
    case 1:
        newtio.c_cflag &= ~CSTOPB;
        break;
    case 2:
        newtio.c_cflag |= CSTOPB;
        break;
    default:
        perror("unsupported stop bits\n");
        return -1;
    }

    if (hardflow)
        newtio.c_cflag |= CRTSCTS;
    else
        newtio.c_cflag &= ~CRTSCTS;

    newtio.c_cc[VTIME] = 20; /* Time-out value (tenths of a second) [!ICANON]. */
    newtio.c_cc[VMIN] = 1;   /* Minimum number of bytes read at once [!ICANON]. */

    tcflush(fd, TCIOFLUSH);

    if (tcsetattr(fd, TCSANOW, &newtio) != 0)
    {
        perror("tcsetattr");
        return -1;
    }

    if (libtty_setcustombaudrate(fd, speed) != 0)
    {
        perror("setbaudrate");
        return -1;
    }

    return 0;
}

void printvec(vector<uint8_t> v)
{
    if (v.size() < 300 || v.size() > 500)
        return;
    int num = 0;
    for (int i = 0; i < v.size(); i++)
    {
        printf(" 0x%.2x", (uint8_t)v[i]);
    }
    for (int i = 9; i < v.size(); i = i + 6)
    {
        int32_t temp = (int32_t)(v[i] << 8 | v[i + 1] << 16 | v[i + 2] << 24) / 256;
        float result = temp / 1000.0f;
        num++;
        cout << result << " ";
    }
    cout << num << endl;
}

void *ToF_Senser(void *arg)
{
    int fd = libtty_open((char *)"/dev/ttyUSB0");
    if (fd < 0)
    {
        cout << "libtty_read error" << endl;
        exit;
    }
    int ret = libtty_setopt(fd, 921600, 8, 1, 'n', 0);
    if (ret != 0)
    {
        cout << "libtty_read error" << endl;
    }
    uint8_t wbuf[] = {0x57, 0x10, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0x63};
    while (1)
    {
        memset(buf, 0, 2048);
        write(fd, wbuf, sizeof(wbuf));
        ret = read(fd, buf, sizeof(buf));
        cout << ret << endl;
        for (int i = 0; i < ret; i++)
        {
            printf(" 0x%.2x", (uint8_t)buf[i]);
        }
        cout << "--------------------" << endl;
        for (int i = 9; i < ret; i = i + 6)
        {
            int32_t temp = (int32_t)((uint8_t)buf[i] << 8 | (uint8_t)buf[i + 1] << 16 | (uint8_t)buf[i + 2] << 24) / 256;
            float result = temp / 1000.0f;
            cout << result << " ";
        }
        cout << "--------------" << endl;
        sleep(1);
    }
}