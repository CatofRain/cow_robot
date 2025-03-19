#include <iostream>
#include <fstream>
#include <vector>
#include <unistd.h>
#include <cmath>
#include "CommApi.h"
#include "proxy/ProxyMotion.h"

using namespace std;

bool connectIPC(Hsc3::Comm::CommApi &cmApi, std::string strIP, uint16_t uPort);
bool disconnectIPC(Hsc3::Comm::CommApi &cmApi);
void loadPosData(GeneralPos &generalPos, double axis0, double axis1, double axis2, double axis3, bool isJoint);
void waitDone(Hsc3::Proxy::ProxyMotion &pMot);
void readCameraPos(std::vector<double> &cameraPos);
void writePos(int robotX, int robotY, int robotZ, std::vector<double> &cameraPos);

std::string Binocular_camera_path = "/home/enqi/cow_robot/yolov5_d435i_detection/Binocular_camera.txt";
vector<double> cameraPos(3, 0);
int step = 20; // 步进 20mm

int main()
{
    Hsc3::Comm::CommApi cmApi("");
    Hsc3::Proxy::ProxyMotion pMot(&cmApi);
    GeneralPos PosMove;

    if (connectIPC(cmApi, "10.10.56.214", 23234))
    {
        pMot.setOpMode(OP_T1);
        pMot.setWorkFrame(0, FRAME_BASE); // FRAME_BASE: 用户坐标系
        pMot.setWorkpieceNum(0, 0);       // setWorkpieceNum(int8_t gpId, int8_t num): gpId 组号，num 工件坐标索引(0..15)

        // 机器人上使能
        pMot.setGpEn(0, true);
        sleep(1);

        for (int y = 300; y <= 420; y += step)
        {
            for (int x = 40; x <= 620; x += step) 
            {
                PosMove.vecPos.clear();
                loadPosData(PosMove, x, y, -40, 0, false);
                pMot.moveTo(0, PosMove, false);
                waitDone(pMot);
                sleep(2);

                readCameraPos(cameraPos);
                writePos(x, y, -40, cameraPos);
            }
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
        printf("CommApi::connect() : ret = %lld\n", ret);
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
    sleep(5);
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
 * @brief 装载数据
 * @param isJoint: 是否关节点，和moveTo()函数的第3个参数(isLinear: 是否直线运动、关节运动)有关
 *                 isJoint是true，则isLinear为false(是关节点，则关节运动)；isJoint是false，则isJoint为true(不是关节点，则直线运动)
 */
/************************************************************************/
void loadPosData(GeneralPos &generalPos, double axis0, double axis1, double axis2, double axis3, bool isJoint)
{
    generalPos.config = 1048576;
    generalPos.isJoint = isJoint; // 是否关节点
    generalPos.ufNum = 0;         // 工件号(修改为对应的用户坐标系的工件坐标)
    generalPos.utNum = -1;        // 工具号
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

/************************************************************************/
/*
 * @brief 等待运动停止，只能用于手动模式下运动到点，用于检测是否处于静止或错误状态
 * @param pMot:运动功能代理
 */
/************************************************************************/
void waitDone(Hsc3::Proxy::ProxyMotion &pMot)
{
    ManState manualState = MAN_STATE_MAX;
    // 此延时是为了防止机器人还未进入运动状态
    sleep(1);

    while (1)
    {
        pMot.getManualStat(manualState);
        if (manualState == MAN_STATE_WAIT || manualState == MAN_STATE_ERROR)
        {
            if (manualState == MAN_STATE_ERROR)
            {
                std::cout << "错误" << std::endl;
            }
            break;
        }
    }
}

void readCameraPos(vector<double> &cameraPos)
{
    ifstream ifs;
    ifs.open(Binocular_camera_path, ios::in);
    if (!ifs.is_open())
    {
        std::cout << "文件打开失败" << std::endl;
        return;
    }

    cameraPos.clear();
    string str;

    getline(ifs, str);

    int j = 0;
    for (int i = 0; i < str.size(); i++)
    {
        if (str[i] == ' ')
        {
            cameraPos.push_back(stod(str.substr(j, i - j)));
            j = i + 1;
        }
    }

    ifs.close();
}

void writePos(int robotX, int robotY, int robotZ, std::vector<double> &cameraPos)
{
    std::ofstream ofs;
    ofs.open(Binocular_camera_path, std::ios::app);

    ofs << robotX << ", " << robotY << ", " << robotZ << " ";

    ofs << static_cast<int>(std::round(cameraPos[0])) << ", "
        << static_cast<int>(std::round(cameraPos[1])) << ", "
        << static_cast<int>(std::round(cameraPos[2])) << std::endl;

    ofs.close();
}
